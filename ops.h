#ifndef OPS_H
#define OPS_H
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef enum { OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_FACT, OP_INVALID } op_t;

typedef struct {
    op_t op;
    int64_t a, b; // b sin uso en factorial
} request_t;

bool parse_request(const char *line, request_t *req, char *errmsg, size_t elen);
bool compute(const request_t *req, int64_t *out, char *errmsg, size_t elen);

#endif
