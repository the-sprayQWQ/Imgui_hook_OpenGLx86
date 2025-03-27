#pragma once
#include <Windows.h>
#include <cstdint>

class inline_hook {
private:
    uintptr_t m_original_address;
    uintptr_t m_self_address;
    uint8_t m_original_byte[5];
    uint8_t m_jmp_byte[5];

public:
    inline_hook(uintptr_t original, uintptr_t self)
        : m_original_address(original), m_self_address(self) {
        // 构造 E9 跳转指令
        m_jmp_byte[0] = 0xE9;
        int32_t offset = static_cast<int32_t>(self - (original + 5));
        memcpy(&m_jmp_byte[1], &offset, 4);

        // 备份原始指令
        DWORD old_protect;
        VirtualProtect(
            reinterpret_cast<void*>(original),
            5,
            PAGE_EXECUTE_READWRITE,
            &old_protect
        );
        memcpy(m_original_byte, reinterpret_cast<void*>(original), 5);
        VirtualProtect(
            reinterpret_cast<void*>(original),
            5,
            old_protect,
            &old_protect
        );
    }

    void apply() {
        DWORD old_protect;
        VirtualProtect(
            reinterpret_cast<void*>(m_original_address),
            5,
            PAGE_EXECUTE_READWRITE,
            &old_protect
        );
        memcpy(reinterpret_cast<void*>(m_original_address), m_jmp_byte, 5);
        VirtualProtect(
            reinterpret_cast<void*>(m_original_address),
            5,
            old_protect,
            &old_protect
        );
    }

    uintptr_t get_original() const {
        return m_original_address;
    }
};