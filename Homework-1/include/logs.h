#pragma once

// ----------------------------- LOGS ----------------------------- //
void LOG_OPEN(int file_code);
void LOG_READ(int file_code);
void LOG_WRITE(int file_code);
void LOG_FORK(int file_code, pid_t child_pid);
void LOG_WAIT(int file_code, int status, int child_index, pid_t child_pid);

// ----------------------------- ERRORS --------------------------- //
void ERROR_MALLOC(int file_code);
void ERROR_OPEN(int file_code);
void ERROR_READ(int file_code);
void ERROR_WRITE(int file_code);
void ERROR_FORK(int file_code);
void ERROR_EXEC(int file_code, int child_index);
void ERROR_WAIT(int file_code, int status, int child_index, pid_t child_pid);