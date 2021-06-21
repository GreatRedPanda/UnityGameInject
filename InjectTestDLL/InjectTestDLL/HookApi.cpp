
#include <Windows.h>
#include <stdint.h>
typedef uint64_t hook_handle64_t;
typedef struct _hook_entry64 {
    // these must be initialized before being passed into the function
    HANDLE proc;          // process to hook in
    uint64_t addr;        // the address to hook
    uint32_t replaceSize; // how many bytes at the hook address to overwrite to make room for our hook

    uint8_t* originalCode;
    uint8_t* redirectCode;
    hook_handle64_t hookData;
    hook_handle64_t remoteCode;
} hook_entry64_t;

const uint8_t sc_x64_redirect[14] = {
    0x52,                                                       // push rdx
    0x48, 0xBA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rdx, PTR_TO_OUR_NEW_CODE
    0xFF, 0xE2,                                                 // jmp rdx
    0x5A                                                        // pop rdx
};

const uint8_t sc_x64_ret[12] = {
    0x48, 0xBA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rdx, PTR_TO_OUR_ORIGINAL_LOC
    0xFF, 0xE2                                                  // jmp rdx
};

int hook_install_x64(hook_entry64_t* ent, uint8_t* shellcode, uint32_t shellcodeSize, uint32_t hookDataSize, void (*prepShellcodeCallback)(uint64_t, uint8_t*)) {
    // ensure minimum replace size
    if (ent->replaceSize < sizeof(sc_x64_redirect)) return 0;

    // prepare our injected code buffer
    uint32_t injectedAsmSize;
    uint8_t* injectedAsm =reinterpret_cast<uint8_t *>( malloc(injectedAsmSize = (shellcodeSize + ent->replaceSize + sizeof(sc_x64_ret))));
    memcpy(injectedAsm, shellcode, shellcodeSize);

    // restore rdx, then put it back on stack before calling the original code here (only needed if rdx is used in the overwritten code)
    // or just provide the option to reserve different registers

    // read original asm to this buffer
    uint8_t* originalAsm = (injectedAsm + shellcodeSize);
    if (!ReadProcessMemory(ent->proc, (void*)ent->addr, originalAsm, ent->replaceSize, NULL)) {
        free(injectedAsm);
        return 0;
    }
    ent->originalCode =reinterpret_cast<uint8_t *>(  malloc(ent->replaceSize));
    memcpy(ent->originalCode, originalAsm, ent->replaceSize);

    // copy return shellcode to the end of the buffer
    uint8_t* returnAsm = (originalAsm + ent->replaceSize);
    memcpy(returnAsm, sc_x64_ret, sizeof(sc_x64_ret));

    // offset to start of addr value for return, cast to 64bit int ptr, and write the addr to return
    // e.g. assign addr to return execution
    *((uint64_t*)(returnAsm + 2)) = ent->addr + sizeof(sc_x64_redirect) - 1; // -1 so we return to the pop which restores rdx

    // if a hook buffer was specified, initialize that memory region with the specified struct
    if (hookDataSize > 0 && prepShellcodeCallback != NULL) {
        ent->hookData = (uint64_t)VirtualAllocEx(ent->proc, NULL, hookDataSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        prepShellcodeCallback(ent->hookData, injectedAsm);
    }

    // allocate memory for our injected code in the external proc
    // this is very easy for anticheats to detect
    ent->remoteCode = (uint64_t)VirtualAllocEx(ent->proc, NULL, injectedAsmSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!ent->remoteCode) {
        free(ent->originalCode);
        goto fail;
    }

    // write the memory to the allocated region
    if (!WriteProcessMemory(ent->proc, (void*)ent->remoteCode, injectedAsm, injectedAsmSize, NULL)) {
        free(ent->originalCode);
        goto fail;
    }

    // prepare our redirect code buffer
    ent->redirectCode = reinterpret_cast<uint8_t*>(malloc(ent->replaceSize));
    memset(ent->redirectCode, 0x90, ent->replaceSize); // fill with nop's
    memcpy(ent->redirectCode, sc_x64_redirect, sizeof(sc_x64_redirect));

    // assign our jmp addr to our injected code addr
    *((uint64_t*)(ent->redirectCode + 3)) = ent->remoteCode;

    // finally write our hook to process memory
    if (!WriteProcessMemory(ent->proc, (void*)ent->addr, ent->redirectCode, ent->replaceSize, NULL)) {
        free(ent->originalCode);
        free(ent->redirectCode);
        free(injectedAsm);
        return 0;
    }

    free(injectedAsm);
    return 1;

fail:
    free(injectedAsm);
    return 0;
}

int hook_uninstall(hook_entry64_t* ent) {
    // read current memory contents at this region
    uint8_t* currentCode = reinterpret_cast<uint8_t*>(malloc(ent->replaceSize));
    if (!ReadProcessMemory(ent->proc, (void*)ent->addr, currentCode, ent->replaceSize, NULL)) {
        goto fail;
    }

    // compare this and what we know we injected to ensure they are the same
    for (uint32_t i = 0; i < ent->replaceSize; ++i) {
        if (currentCode[i] != ent->redirectCode[i]) {
            goto fail;
        }
    }

    // memory passed integrity check, restore original contents
    if (!WriteProcessMemory(ent->proc, (void*)ent->addr, ent->originalCode, ent->replaceSize, NULL)) {
        goto fail;
    }

    // attempt to free remote hook resources
    VirtualFreeEx(ent->proc, (void*)ent->hookData, 0, MEM_DECOMMIT | MEM_RELEASE);
    VirtualFreeEx(ent->proc, (void*)ent->remoteCode, 0, MEM_DECOMMIT | MEM_RELEASE);

    hook_free(ent);
    free(currentCode);
    return 1;

fail:
    free(currentCode);
    return 0;
}

void hook_free(hook_entry64_t* ent) {
    free(ent->originalCode);
    free(ent->redirectCode);
}