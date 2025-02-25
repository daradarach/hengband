#pragma once
/*!
 * @file main-win-utils.h
 * @brief Windows版固有実装(ユーティリティー)ヘッダ
 */

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <windows.h>

/*!
 * @brief マルチバイト文字列(CP932)をワイド文字列へ変換するクラス
 */
class to_wchar {
public:
    to_wchar(const char *src)
    {
        if (!src) {
            return;
        }

        int size = ::MultiByteToWideChar(932, 0, src, -1, NULL, 0);
        if (size > 0) {
            buf = std::vector<WCHAR>(size + 1);
            if (::MultiByteToWideChar(932, 0, src, -1, (*buf).data(), (*buf).size()) == 0) {
                // fail
                buf = std::nullopt;
            }
        }
    }

    virtual ~to_wchar() = default;

    to_wchar(const to_wchar &) = delete;
    to_wchar &operator=(const to_wchar &) = delete;

    WCHAR *wc_str()
    {
        return buf ? (*buf).data() : NULL;
    }

protected:
    std::optional<std::vector<WCHAR>> buf;
};

/*!
 * @brief ワイド文字列をマルチバイト文字列(CP932)へ変換するクラス
 */
class to_multibyte {
public:
    to_multibyte(const WCHAR *src)
    {
        if (!src) {
            return;
        }

        int size = ::WideCharToMultiByte(932, 0, src, -1, NULL, 0, NULL, NULL);
        if (size > 0) {
            buf = std::vector<char>(size + 1);
            if (::WideCharToMultiByte(932, 0, src, -1, (*buf).data(), (*buf).size(), NULL, NULL) == 0) {
                // fail
                buf = std::nullopt;
            }
        }
    }

    virtual ~to_multibyte() = default;

    to_multibyte(const to_multibyte &) = delete;
    char *&operator=(const char *&) = delete;

    char *c_str()
    {
        return buf ? (*buf).data() : NULL;
    }

protected:
    std::optional<std::vector<char>> buf;
};

bool is_already_running();
void save_screen_as_html(HWND hWnd);
void open_dir_in_explorer(const std::filesystem::path &path);
std::optional<std::filesystem::path> get_open_filename(OPENFILENAMEW *ofn, const std::filesystem::path &path_dir, const std::filesystem::path &path_file, DWORD max_name_size);
