#include "ops.h"
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

static bool will_mul_overflow(int64_t a, int64_t b){
    if (a == 0 || b == 0) return false;
    __int128 r = (__int128)a * (__int128)b;
    return (r > INT64_MAX || r < INT64_MIN);
}

bool parse_request(const char *line, request_t *req, char *errmsg, size_t elen){
    char opch=0;
    int scanned=0;
    int64_t a=0,b=0;
    // Trim leading spaces
    while (*line==' '||*line=='\t') line++;
    if (sscanf(line, " %c %n", &opch, &scanned) != 1) {
        snprintf(errmsg, elen, "formato");
        return false;
    }
    line += scanned;
    if (opch=='!'){
        if (sscanf(line, "%" SCNd64, &a) != 1){
            snprintf(errmsg, elen, "formato factorial");
            return false;
        }
        req->op = OP_FACT; req->a = a; req->b = 0;
        return true;
    }
    if (sscanf(line, "%" SCNd64 " %" SCNd64, &a, &b) != 2){
        snprintf(errmsg, elen, "formato binario");
        return false;
    }
    switch(opch){
        case '+': req->op=OP_ADD; break;
        case '-': req->op=OP_SUB; break;
        case '*': req->op=OP_MUL; break;
        case '/': req->op=OP_DIV; break;
        default: req->op=OP_INVALID; snprintf(errmsg, elen, "operacion"); return false;
    }
    req->a=a; req->b=b;
    return true;
}

bool compute(const request_t *req, int64_t *out, char *errmsg, size_t elen){
    switch(req->op){
        case OP_ADD: {
            __int128 r=(__int128)req->a + (__int128)req->b;
            if (r>INT64_MAX || r<INT64_MIN){ snprintf(errmsg,elen,"overflow"); return false; }
            *out=(int64_t)r; return true;
        }
        case OP_SUB: {
            __int128 r=(__int128)req->a - (__int128)req->b;
            if (r>INT64_MAX || r<INT64_MIN){ snprintf(errmsg,elen,"overflow"); return false; }
            *out=(int64_t)r; return true;
        }
        case OP_MUL:
            if (will_mul_overflow(req->a, req->b)){ snprintf(errmsg,elen,"overflow"); return false; }
            *out=req->a * req->b; return true;
        case OP_DIV:
            if (req->b==0){ snprintf(errmsg,elen,"division por cero"); return false; }
            *out=req->a / req->b; return true; // división entera
        case OP_FACT: {
            if (req->a < 0){ snprintf(errmsg,elen,"factorial de negativo"); return false; }
            // 20! cabe en uint64, no en int64; limitamos a 20 para seguridad
            if (req->a > 20){ snprintf(errmsg,elen,"n demasiado grande"); return false; }
            unsigned __int128 acc=1;
            for (int64_t i=2;i<=req->a;i++) acc*=i;
            if (acc > (unsigned __int128)INT64_MAX){ snprintf(errmsg,elen,"overflow"); return false; }
            *out=(int64_t)acc; return true;
        }
        default: snprintf(errmsg,elen,"op invalida"); return false;
    }
}
