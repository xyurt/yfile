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

    // Deletes the file specified by filename.
    // Returns 0 on success, -1 on failure.
    int file_delete(const char *filename) {
        return DeleteFileA(filename) ? 0 : -1;
    }

    // Securely deletes a file by overwriting its contents with zeros before deleting it.
    // buffer_length specifies the size of the buffer used for writing zeros in chunks.
    // Returns 0 on success, -1 on failure.
    int file_secure_delete_ex(const char *filename, size_t buffer_length) {
        if (filename == NULL) { return -1; }

        // Open file for read/write binary, using UTF-8 aware open (you presumably have this function).
        FILE *fp = file_open_utf8(filename, "r+b");
        if (fp == NULL) { return -1; }

        // Get the size of the file for overwriting.
        int64_t filesz64 = file_get_size(fp);
        if (filesz64 < 0) { file_close(fp); return -1; }

        // If file is empty, just close and delete it.
        if (filesz64 == 0) { file_close(fp); file_delete(filename); return 0; }

        // Safely cast file size to size_t (assuming file < 4GB or 64-bit size_t).
        size_t filesz = (size_t)filesz64;

        // Buffer size to use for zeroing out the file.
        size_t buflen = filesz > buffer_length ? buffer_length : filesz;

        // Allocate a buffer filled with zeros.
        char *tempbuf = (char *)malloc(sizeof(char) * buflen);
        if (tempbuf == NULL) { file_close(fp); return -1; }
        memset(tempbuf, 0, buflen);

        // Calculate how many full chunks we need to write.
        size_t loopcount = (size_t)(filesz / buflen);

        // Reset file pointer to beginning.
        if (file_set_offset(fp, 0) != 0) { free(tempbuf); file_close(fp); return -1; }

        // Overwrite file chunk by chunk with zeros.
        for (size_t i = 0; i < loopcount; i++) {
            // file_write returns 0 on failure, so if that happens return -1.
            if (file_write(fp, tempbuf, buflen) == 0) {
                free(tempbuf);
                file_close(fp);
                return -1;
            }
        }
        free(tempbuf);

        // Handle remaining bytes if file size not divisible by buffer length.
        size_t remainingbytes = filesz - (loopcount * buflen);
        if (remainingbytes > 0) {
            // Allocate smaller buffer for remaining bytes.
            char *tempbuf_rem = (char *)malloc(sizeof(char) * remainingbytes);
            if (tempbuf_rem == NULL) { file_close(fp); return -1; }
            memset(tempbuf_rem, 0, remainingbytes);

            // Write the remaining zero bytes.
            if (file_write(fp, tempbuf_rem, remainingbytes) == 0) {
                free(tempbuf_rem);
                file_close(fp);
                return -1;
            }
            free(tempbuf_rem);
        }

        // Flush buffer to disk to ensure data is written.
        if (file_flush(fp) != 0) { file_close(fp); return -1; }

        // Close the file.
        if (file_close(fp) == -1) { return -1; }

        // Finally delete the file after overwriting.
        return file_delete(filename);
    }

    // Sets the file position of the given FILE* fp to offset relative to origin (SEEK_SET, SEEK_CUR, SEEK_END).
    // Returns 0 on success, -1 on failure.
    int file_set_offset_ex(FILE *fp, int64_t offset, int origin) {
        // Check valid origin and fp not NULL.
        if ((origin != SEEK_CUR && origin != SEEK_END && (origin != SEEK_SET || offset < 0)) || fp == NULL) { return -1; }

        // _fseeki64 is Windows-specific 64-bit file seek function.
        return _fseeki64(fp, offset, origin) != 0 ? -1 : 0;
    }

    // Sets file offset to an absolute position offset bytes from start (SEEK_SET).
    // Returns 0 on success, -1 on failure.
    int file_set_offset(FILE *fp, int64_t offset) {
        if (offset < 0) { return -1; }
        return file_set_offset_ex(fp, offset, SEEK_SET);
    }

    // Returns the current file offset in bytes or -1 on failure.
    int64_t file_get_offset(FILE *fp) {
        if (fp == NULL) { return -1LL; }
        return _ftelli64(fp);
    }

    // Gets the total size of the file in bytes or -1 on failure.
    // It saves the current position, seeks to end to get size, then restores position.
    int64_t file_get_size(FILE *fp) {
        if (fp == NULL) { return -1; }
        int64_t current, size;
        if ((current = file_get_offset(fp)) == -1) { return -1; }
        if (file_set_offset_ex(fp, 0, SEEK_END) != 0) { return -1; }
        if ((size = file_get_offset(fp)) == -1) { return -1; }
        if (file_set_offset(fp, current) != 0) { return -1; }
        return size;
    }

    // Resets file pointer to beginning using standard rewind.
    // Void because no return value needed.
    void file_rewind(FILE *fp) {
        if (fp == NULL) { return; }
        rewind(fp);
    }

    // Returns non-zero if end-of-file has been reached, zero otherwise.
    // Returns 1 if fp is NULL (considered EOF for safety).
    int file_eof(FILE *fp) {
        if (fp == NULL) { return 1; }
        return feof(fp);
    }

    // Truncates or extends the file to specified size in bytes.
    // Returns 0 on success, -1 on failure.
    // Uses Windows HANDLE from FILE* for SetEndOfFile API.
    int file_truncate(FILE *fp, int64_t size) {
        if (fp == NULL) { return -1; }
        HANDLE h;
        if ((h = file_get_handle(fp)) == INVALID_HANDLE_VALUE) { return -1; }
        if (file_set_offset(fp, size) != 0) { return -1; }
        return SetEndOfFile(h) ? 0 : -1;
    }

    // Checks if given filename is a directory.
    // Returns 0 if directory, 1 if not, -1 on error (like file not existing).
    int file_is_directory(const char *filename) {
        if (filename == NULL) { return -1; }
        return file_has_attributes(filename, FILE_ATTRIBUTE_DIRECTORY);
    }

    // Creates a directory specified by partial_path with optional security attributes.
    // Returns 0 if directory created successfully, 1 if failed (or directory exists?).
    int create_directory_part_ex(const char *partial_path, LPSECURITY_ATTRIBUTES attributes) {
        return CreateDirectoryA(partial_path, attributes) != 0 ? 0 : 1;
    }

    // Wrapper for create_directory_part_ex without security attributes.
    int create_directory_part(const char *partial_path) {
        return create_directory_part_ex(partial_path, NULL);
    }

    // Ensures that the entire directory path exists by creating any missing directories.
    // Returns 0 on success, -1 on failure.
    // Supports paths like "C:\\folder1\\folder2\\folder3"
    int file_ensure_directory_ex(const char *path, LPSECURITY_ATTRIBUTES attributes) {
        if (path == NULL || path[0] == '\0') { return -1; }

        // Copy path to mutable buffer.
        size_t pathlen = strlen(path);
        char *tmp = (char *)malloc(sizeof(char) * (pathlen + 1));
        if (tmp == NULL) { return -1; }
        memcpy(tmp, path, pathlen);
        tmp[pathlen] = '\0';

        // Remove trailing slash/backslash if exists.
        if (tmp[pathlen - 1] == '\\' || tmp[pathlen - 1] == '/') {
            tmp[pathlen - 1] = '\0';
        }

        // Iterate through tmp string, create directories progressively.
        for (char *p = tmp + 1; *p; ++p) {
            if (*p != '\\' && *p != '/') { continue; }
            *p = '\0';                          // Temporarily terminate string here.
            create_directory_part_ex(tmp, attributes);  // Create directory if missing.
            *p = '\\';                         // Restore slash.
        }

        // Finally create last directory in path.
        int ret = create_directory_part(tmp);

        free(tmp);

        // If directory already exists, treat as success.
        if (ret != 0) {
            if (file_last_error_is(ERROR_ALREADY_EXISTS)) {
                return 0;
            }
            return -1;
        }

        return 0;
    }

    // Returns 0 on success, non-zero on failure.
    int file_flush(FILE *fp) {
        if (fp == NULL) return -1;
        return fflush(fp);
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
