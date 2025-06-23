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
        if (!filename || !mode || !mode[0]) return NULL;
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

    // Sets the file offset for the given FILE pointer using a custom origin and 64-bit offset.
    // 
    // @param fp     A valid file pointer.
    // @param offset 64-bit offset value. Must be non-negative if SEEK_SET is used.
    // @param origin Can be SEEK_SET, SEEK_CUR, or SEEK_END.
    // @return 0 on success, -1 on failure.
    int file_set_offset_ex(FILE *fp, int64_t offset, int origin) {
        // Validate input: disallow negative offset with SEEK_SET, check null pointer.
        if ((origin != SEEK_CUR && origin != SEEK_END && (origin != SEEK_SET || offset < 0)) || fp == NULL)
            return -1;

        // Use _fseeki64 for 64-bit file offset support.
        return _fseeki64(fp, offset, origin) != 0 ? -1 : 0;
    }

    // Sets the file offset absolutely (from beginning of the file).
    //
    // @param fp     A valid file pointer.
    // @param offset Absolute offset from the beginning. Must be >= 0.
    // @return 0 on success, -1 on failure.
    int file_set_offset(FILE *fp, int64_t offset) {
        if (offset < 0) return -1;
        return file_set_offset_ex(fp, offset, SEEK_SET);
    }

    // Gets the current 64-bit file offset of the provided FILE pointer.
    //
    // @param fp A valid file pointer.
    // @return Current offset as int64_t, or -1 on error.
    int64_t file_get_offset(FILE *fp) {
        if (fp == NULL) return -1LL;
        return _ftelli64(fp);
    }

    // Writes a buffer to the file with 64-bit safety and handles partial writes.
    //
    // @param fp   A valid file pointer opened for writing.
    // @param buf  Pointer to the data buffer to write.
    // @param len  Number of bytes to write.
    // @return Number of bytes written; returns 0 on failure.
    size_t file_write(FILE *fp, const char *buf, size_t len) {
        if (fp == NULL || buf == NULL || len == 0) return 0;

        size_t total = 0;

        // Write in a loop to handle partial writes (especially relevant for pipes or slow I/O).
        while (total < len) {
            size_t written = fwrite(buf + total, sizeof(char), len - total, fp);

            // fwrite returns 0 on error or if no data was written
            if (written == 0) {
                if (ferror(fp)) {
                    return 0;
                }
                break;
            }

            total += written;
        }

        return total;
    }

    // Reads up to max_len bytes from a file into a buffer.
    //
    // @param fp      A valid file pointer opened for reading.
    // @param buf     Destination buffer where read data will be stored.
    // @param max_len Maximum number of bytes to read.
    // @return Number of bytes successfully read; returns 0 on error.
    size_t file_read(FILE *fp, char *buf, size_t max_len) {
        if (fp == NULL || buf == NULL || max_len == 0) return 0;
        size_t read = fread(buf, 1, max_len, fp);
        if (ferror(fp)) return 0;
        return read;
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
