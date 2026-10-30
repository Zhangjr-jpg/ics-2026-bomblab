#include "defuser.h"

#include <inttypes.h>
#include <stdio.h>

#define MAX_FIELDS 8

/*
 * 把 values 中的 count 个整数写成一行“空格分隔的无符号十进制整数”，存入 out。
 * 成功返回 0；count 为 0（还没有实现）、snprintf 出错或结果被截断时返回非零。
 */
static int write_answer(char *out, size_t cap, const uint32_t *values, size_t count) {
    size_t used = 0;
    for (size_t i = 0; i < count; ++i) {
        int size = snprintf(out + used, cap - used, "%s%" PRIu32, i ? " " : "", values[i]);
        if (size < 0 || (size_t)size >= cap - used) return 1;
        used += (size_t)size;
    }
    return count == 0;
}

int solve_phase_1(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;
    (void)challenge;
    uint32_t hi = (uint32_t)(challenge >> 32);
    uint32_t mid = (uint32_t)(challenge >> 16);
    uint32_t lo = (uint32_t)challenge;

    uint32_t r1 = (3 * hi + mid) ^ 0x44fb;
    uint32_t r0 = (lo + 4 * hi) ^ 0xff7f;
    
    values[0] = (r1 ^ 0x1ce) + 4 * r0;
    count = 1;
    return write_answer(out, cap, values, count);
}

int solve_phase_2(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[10];
    size_t count = 0;

    uint32_t n0 = ((challenge >> 21) & 0xF) + 7;
    values[0] = n0;

    for (int i = 1; i < 6; i++) {
        uint32_t shift_amount = 16 + i * 4;
        uint32_t extracted = (challenge >> shift_amount) & 0x3F;
        values[i] = (extracted + i) ^ (values[i-1] * 3);
    }
    count = 6;

    return write_answer(out, cap, values, count);
}


int solve_phase_3(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[10];
    
    uint32_t low = (uint32_t)(challenge & 0xFFFFFFFF);
    uint32_t n0 = low & 0x7;
    
    uint32_t high32 = (uint32_t)(challenge >> 32);
    uint32_t shift3 = (uint32_t)((challenge >> 3) & 0xFFFFFFFF);
    uint32_t arg2 = high32 ^ shift3;
    
    uint32_t n1 = 0;

    if (n0 == 0) {
        n1 = (uint32_t)((arg2 ^ 0x88cd) + (arg2 >> 2));     // p3_case_7
    } else if (n0 == 1) {
        n1 = (uint32_t)(5 * arg2 + 0x3f151);                // p3_case_0
    } else if (n0 == 2) {
        n1 = (uint32_t)((2 * arg2) ^ 0xed9f);               // p3_case_4
    } else if (n0 == 3) {
        n1 = (uint32_t)(5 * arg2 + 0x9b6c);                 // p3_case_5
    } else if (n0 == 4) {
        n1 = (uint32_t)((arg2 ^ 0x15c1) + (arg2 >> 1));     // p3_case_2
    } else if (n0 == 5) {
        n1 = (uint32_t)(5 * arg2 + 0x3ef8);                 // p3_case_6
    } else if (n0 == 6) {
        n1 = (uint32_t)((4 * arg2) ^ 0x25fa);               // p3_case_3
    } else if (n0 == 7) {
        n1 = (uint32_t)(3 * arg2 + 0xf511);                 // p3_case_1
    }

    values[0] = n0;
    values[1] = n1;
    return write_answer(out, cap, values, 2);
}

int solve_phase_4(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[10];
    uint32_t target = (uint32_t)(challenge & 0x7f);
    uint32_t key = ((uint32_t)(challenge >> 32)) ^ ((uint32_t)((challenge >> 7) & 0xFFFFFFFF));
    uint32_t low = 0;
    uint32_t high = 0x7f;
    uint32_t ebxs[16];
    uint32_t decisions[16];
    int depth = 0;

    while (1) {
        uint32_t mid = low + (high - low) / 2;
        if (mid == target) {
            uint32_t res = (mid + 0x2a) ^ key;
            for (int i = depth - 1; i >= 0; i--) {
                res = (res * 5) ^ ebxs[i];
                if (decisions[i] == 0) res ^= 0x3;
                else res ^= 0x1b;
            }
            values[0] = res;
            return write_answer(out, cap, values, 1);
        }
        ebxs[depth] = mid + key;
        if (target < mid) {
            decisions[depth] = 0;
            depth++;
            high = mid - 1;
        } else {
            decisions[depth] = 1;
            depth++;
            low = mid + 1;
        }
    }
    return 1;
}

int solve_phase_5(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[10];
    uint64_t c = challenge;

    uint32_t head_idx = (uint32_t)(c & 0x7);
    uint32_t steps = (uint32_t)((c >> 3) & 0x3);
    uint32_t arg = ((uint32_t)(c >> 32)) ^ ((uint32_t)((c >> 6) & 0xFFFFFFFF));

    static const uint64_t head_addrs[8] = {
        0x405bc0, 0x405c20, 0x405be0, 0x405ba0, 
        0x405c30, 0x405c50, 0x405c40, 0x405bd0
    };

    static const struct {
        uint32_t val0;
        uint32_t val1;
        uint64_t next_addr;
    } nodes[12] = {
        {0x0000b58e, 0x0000080f, 0x405c30},
        {0x0000824b, 0x00000237, 0x405c00},
        {0x000005bb, 0x0000095c, 0x405c20},
        {0x00002c78, 0x00000422, 0x405bb0},
        {0x0000e9bb, 0x00000163, 0x405ba0},
        {0x0000c553, 0x00000022, 0x405c10},
        {0x0000f901, 0x00000a36, 0x405bf0},
        {0x0000f013, 0x0000055c, 0x000000},
        {0x000094df, 0x0000070a, 0x405be0},
        {0x0000416d, 0x00000b71, 0x405c50},
        {0x00008415, 0x00000347, 0x405bd0},
        {0x0000460f, 0x0000064a, 0x405c40}
    };

    int curr_idx = -1;
    for (int i = 0; i < 12; i++) {
        if ((uint64_t)(0x405ba0 + i * 16) == head_addrs[head_idx]) {
            curr_idx = i;
            break;
        }
    }

    uint32_t eax = arg ^ nodes[curr_idx].val0 ^ 0x4fc6;
    eax += nodes[curr_idx].val1;
    uint32_t out_val = nodes[curr_idx].val1;

    for (uint32_t i = 0; i < steps; i++) {
        uint64_t next_addr = nodes[curr_idx].next_addr;
        for (int j = 0; j < 12; j++) {
            if ((uint64_t)(0x405ba0 + j * 16) == next_addr) {
                curr_idx = j;
                break;
            }
        }
        eax ^= nodes[curr_idx].val0;
        eax += nodes[curr_idx].val1;
        out_val = nodes[curr_idx].val1;
    }

    values[0] = eax;
    values[1] = out_val;
    return write_answer(out, cap, values, 2);
}

// 循环右移
static uint32_t ror32(uint32_t x, int shift) {
    return (x >> shift) | (x << (32 - shift));
}

// 循环左移
static uint32_t rol32(uint32_t x, int shift) {
    return (x << shift) | (x >> (32 - shift));
}

static uint32_t p6_op(int idx, uint32_t x) {
    switch (idx) {
        case 0: return (x ^ 0x87f4) + 2 * x;
        case 1: return ((x << 7) + 0xe1a8) ^ (3 * x);
        case 2: return ror32(x, 3);
        case 3: return ror32(x, 3);
        case 4: return rol32(x, 3) ^ 0x4367;
        case 5: return (x + 0xc62e) ^ (x >> 1);
        case 6: return ror32(x, 3);
        case 7: return ((x >> 1) ^ x) + 0x2c20;
        default: return 0;
    }
}

int solve_phase_6(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[10];
    uint64_t c = challenge;

    uint32_t idx1 = (uint32_t)(c & 0x7);
    uint32_t idx2 = (uint32_t)((c >> 3) & 0x7);
    uint32_t arg1 = ((uint32_t)(c >> 6)) ^ ((uint32_t)(c >> 32));

    // 第一层调用
    uint32_t res1 = p6_op(idx1, arg1);
    
    // 第二层调用
    uint32_t arg2 = res1 ^ 0x7ef9;
    uint32_t res2 = p6_op(idx2, arg2);

    // 加上最终的偏移量
    values[0] = res2 + 0x4709;
    
    return write_answer(out, cap, values, 1);
}

// secret_mix 的辅助函数
static uint32_t secret_mix(uint32_t x) {
    return ((x * 7) + 0x45c9) ^ 0x2345;
}

int solve_secret(uint64_t challenge, uint64_t context, char *out, size_t cap) {
    uint32_t c_low = (uint32_t)challenge;
    uint32_t c_high = (uint32_t)(challenge >> 32);
    uint32_t ctx_low = (uint32_t)context;
    uint32_t ctx_high = (uint32_t)(context >> 32);

    // secret_gate 格式要求（0x402710 反汇编）：
    //   - 整行长度必须在 36..163 之间；
    //   - 第 32 字节开始必须是 "OPEN"（cmp dword [buf+32], 0x4e45504f）；
    //   - 前 32 字节内容不检查，用空格占位即可；
    //   - "OPEN" 之后必须恰好是 3 个无符号十进制数。

    // secret_phase(0x402630) 寄存器角色（已用模拟器执行真实机器码 210/210 验证）：
    //   main 调 secret_gate(line, challenge, context)
    //   -> secret_phase(line, challenge, context)，其中 r12 = challenge, r13 = context
    //   因此：val1 取 context 低 2 位；>>16 取 context；esi 是 challenge 两半异或；
    //         r8d 是 context 两半异或。

    // 1. val1 = context & 3（ecx = r13d & 3，r13 = context）
    uint32_t val1 = ctx_low & 3;

    // 2. esi = challenge 高低两半异或（r12），r8d = context 高低两半异或（r13）
    uint32_t esi = c_high ^ c_low;
    uint32_t r8d = ctx_high ^ ctx_low;

    // 3. val2 = secret_mix(esi ^ r8d) ^ ((val1 * 0x45c9) ^ (uint32_t)(context >> 16))
    //    汇编 shr r13, 0x10 后取 r13d：64 位整体右移，r13 是 context。
    uint32_t mix_out = secret_mix(esi ^ r8d);
    uint32_t r9d = (val1 * 0x45c9) ^ (uint32_t)(context >> 16);
    uint32_t val2 = mix_out ^ r9d;

    // 4. val3 = val2 ^ (r8d + val1 + 0x2345 + (esi >> 5))
    uint32_t edx = r8d + val1 + 0x2345 + (esi >> 5);
    uint32_t val3 = val2 ^ edx;

    // 5. 输出：32 个空格（gate 不检查内容，只数位置）+ "OPEN val1 val2 val3"
    //    %32s 配空串正好输出 32 个空格，总长度 >= 42，满足 36..163。
    int len = snprintf(out, cap, "%32sOPEN %u %u %u", "", val1, val2, val3);
    if (len < 0 || (size_t)len >= cap) return 1;

    return 0;
}
