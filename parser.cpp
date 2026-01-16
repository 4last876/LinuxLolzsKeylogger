#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <xkbcommon/xkbcommon.h>

int main() {
    std::string layout;
    std::string filename;

    std::cout << "Введите код языка (например, 'us' или 'ru'): ";
    std::cin >> layout;
    
    std::cout << "Введите имя файла с кодами (например, 'input.txt'): ";
    std::cin >> filename;


    std::ifstream inputFile(filename);
    if (!inputFile.is_open()) {
        std::cerr << "Ошибка: Не удалось открыть файл " << filename << std::endl;
        return 1;
    }

    std::vector<uint32_t> keycodes;
    uint32_t code;
    while (inputFile >> code) {
        keycodes.push_back(code);
    }
    inputFile.close();

    if (keycodes.empty()) {
        std::cerr << "Файл пуст или содержит некорректные данные." << std::endl;
        return 1;
    }

    struct xkb_context *ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    if (!ctx) return 1;

    struct xkb_rule_names names = {
        .rules = "evdev",
        .model = "pc105",
        .layout = layout.c_str(),
        .variant = nullptr,
        .options = nullptr
    };

    struct xkb_keymap *keymap = xkb_keymap_new_from_names(ctx, &names, XKB_KEYMAP_COMPILE_NO_FLAGS);
    if (!keymap) {
        std::cerr << "Ошибка: Не удалось создать карту для раскладки: " << layout << std::endl;
        xkb_context_unref(ctx);
        return 1;
    }

    struct xkb_state *state = xkb_state_new(keymap);


    std::cout << "\nРезультат расшифровки (" << layout << "):\n";
    for (uint32_t kc : keycodes) {
        char buffer[16]; 
        if (xkb_state_key_get_utf8(state, kc + 8, buffer, sizeof(buffer)) > 0) {
            std::cout << buffer;
        }
    }
    std::cout << std::endl;

    xkb_state_unref(state);
    xkb_keymap_unref(keymap);
    xkb_context_unref(ctx);

    return 0;
}
