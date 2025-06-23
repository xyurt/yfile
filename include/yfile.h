#ifndef YFILE_YFILE_
#define YFILE_YFILE_

#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <io.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FILE_SUCCESS 0
#define FILE_ERROR   -1
#define FILE_FALSE    1

    /**
     * @brief Checks if a file has the specified attributes.
     * @param filename The path to the file.
     * @param attributes Bitmask of attributes to check (e.g., FILE_ATTRIBUTE_DIRECTORY).
     * @return 0 if all attributes match, 1 if not, -1 on error.
     */
    int file_has_attributes(const char *filename, unsigned long attributes) {
        if (filename == NULL) return -1;
        unsigned long attr = GetFileAttributesA(filename);
        return attr == INVALID_FILE_ATTRIBUTES ? -1 : ((attr & attributes) == attributes ? 0 : 1);
    }

    /**
     * @brief Sets file attributes.
     * @param filename The path to the file.
     * @param attributes The attribute flags to set.
     * @return 0 on success, -1 on failure.
     */
    int file_set_attributes(const char *filename, unsigned long attributes) {
        if (filename == NULL) return -1;
        return SetFileAttributesA(filename, attributes) ? 0 : -1;
    }

    /**
     * @brief Checks if a file exists.
     * @param filename Path to check.
     * @return 0 if file exists, 1 if not.
     */
    int file_exists(const char *filename) {
        return GetFileAttributesA(filename) != INVALID_FILE_ATTRIBUTES ? 0 : 1;
    }

    /**
     * @brief Checks if a file is accessible (i.e., exists and can be opened).
     * @param filename File path.
     * @return 0 if accessible, 1 if not, -1 on invalid input.
     */
    int file_accessible(const char *filename) {
        if (filename == NULL) return -1;
        return GetFileAttributesA(filename) == INVALID_FILE_ATTRIBUTES ? 1 : 0;
    }

    /**
     * @brief Opens a file using standard fopen.
     * @param filename File path.
     * @param mode fopen-style mode string.
     * @return FILE pointer on success, NULL on failure.
     */
    FILE *file_open(const char *filename, const char *mode) {
        if (!filename || !mode || !mode[0]) return NULL;
        return fopen(filename, mode);
    }

    /**
     * @brief Opens a UTF-8 encoded file using wide-character Windows API.
     * @param filename UTF-8 encoded file path.
     * @param mode UTF-8 encoded mode string (e.g., "r", "w").
     * @return FILE pointer on success, NULL on failure.
     */
    FILE *file_open_utf8(const char *filename, const char *mode) {
        if (!filename || !mode || !mode[0] || (strchr(mode, 'r') && file_accessible(filename))) return NULL;
        int wlen_path = MultiByteToWideChar(CP_UTF8, 0, filename, -1, NULL, 0);
        if (!wlen_path) return NULL;
        wchar_t *wpath = (wchar_t *)malloc(sizeof(wchar_t) * wlen_path);
        if (!wpath) return NULL;
        MultiByteToWideChar(CP_UTF8, 0, filename, -1, wpath, wlen_path);

        int wlen_mode = MultiByteToWideChar(CP_UTF8, 0, mode, -1, NULL, 0);
        wchar_t *wmode = (wchar_t *)malloc(sizeof(wchar_t) * wlen_mode);
        if (!wmode) { free(wpath); return NULL; }
        MultiByteToWideChar(CP_UTF8, 0, mode, -1, wmode, wlen_mode);

        FILE *fp = _wfopen(wpath, wmode);
        free(wpath);
        free(wmode);
        return fp;
    }

    /**
     * @brief Closes a file.
     * @param fp Pointer to FILE.
     * @return 0 on success, -1 on error.
     */
    int file_close(FILE *fp) {
        return (fp == NULL) ? -1 : (fclose(fp) == 0 ? 0 : -1);
    }

    /**
     * @brief Gets the Windows HANDLE from a FILE*.
     * @param fp Pointer to FILE.
     * @return Valid HANDLE on success, INVALID_HANDLE_VALUE on failure.
     */
    HANDLE file_get_handle(FILE *fp) {
        if (fp == NULL) return INVALID_HANDLE_VALUE;
        return (HANDLE)_get_osfhandle(_fileno(fp));
    }

    /**
     * @brief Locks the file for exclusive/shared access.
     * @param fp Pointer to FILE.
     * @param exclusive 1 for exclusive (write), 0 for shared (read).
     * @return 0 on success, -1 on error.
     */
    int file_lock(FILE *fp, int exclusive) {
        if (fp == NULL) return -1;
        HANDLE hFile = file_get_handle(fp);
        if (hFile == INVALID_HANDLE_VALUE) return -1;
        OVERLAPPED ov = { 0 };
        return LockFileEx(hFile, exclusive ? LOCKFILE_EXCLUSIVE_LOCK : 0, 0, MAXDWORD, MAXDWORD, &ov) ? 0 : -1;
    }

    /**
     * @brief Unlocks the file.
     * @param fp Pointer to FILE.
     * @return 0 on success, -1 on failure.
     */
    int file_unlock(FILE *fp) {
        if (fp == NULL) return -1;
        HANDLE hFile = file_get_handle(fp);
        if (hFile == INVALID_HANDLE_VALUE) return -1;
        OVERLAPPED ov = { 0 };
        return UnlockFileEx(hFile, 0, MAXDWORD, MAXDWORD, &ov) ? 0 : -1;
    }

    /**
     * @brief Copies a file.
     * @param src Source path.
     * @param dst Destination path.
     * @param fail_if_exists 1 to fail if dst exists, 0 to overwrite.
     * @return 0 on success, -1 on error.
     */
    int file_copy_ex(const char *src, const char *dst, int fail_if_exists) {
        if (!src || !dst) return -1;
        return CopyFileA(src, dst, fail_if_exists) ? 0 : -1;
    }

    /**
     * @brief Copies a file (fails if destination exists).
     * @param src Source.
     * @param dst Destination.
     * @return 0 on success, -1 on failure.
     */
    int file_copy(const char *src, const char *dst) {
        return file_copy_ex(src, dst, 1);
    }

    /**
     * @brief Moves a file.
     * @param src Source path.
     * @param dst Destination path.
     * @return 0 on success, -1 on error.
     */
    int file_move(const char *src, const char *dst) {
        return MoveFileA(src, dst) ? 0 : -1;
    }

    /**
     * @brief Deletes a file.
     * @param filename File path.
     * @return 0 on success, -1 on failure.
     */
    int file_delete(const char *filename) {
        return DeleteFileA(filename) ? 0 : -1;
    }

    /**
     * @brief Gets the last WinAPI error.
     * @return Error code.
     */
    unsigned long file_last_error() {
        return GetLastError();
    }

    /**
     * @brief Compares the last WinAPI error with a given code.
     * @param err Error code to compare.
     * @return 0 if equal, 1 if not.
     */
    int file_last_error_is(int err) {
        return file_last_error() == (unsigned long)err ? 0 : 1;
    }

#ifdef __cplusplus
}
#endif

#endif // FILE_UTILS_H
