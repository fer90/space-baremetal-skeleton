# Architecture

Bare-metal RISC-V firmware on QEMU `virt`, built around FreeRTOS and a small set of cooperating tasks. This document explains how the pieces fit together and how to extend the skeleton without fighting the layout.

## Boot and scheduler

```
startup.s  →  main()  →  fault_queue_init, command_init, memory_protection_init
                         →  tasks_create_all()  →  vTaskStartScheduler()
```

`startup.s` zeroes BSS, sets up a 4 KB ISR/trap stack, and jumps to `main`. `main` initializes IPC queues and the software memory-protection table, registers the critical-text region, creates all application tasks, then starts the FreeRTOS scheduler. Any fatal init failure calls `system_halt()` — the kernel never starts partially.

## Task table

All application tasks are declared once in `task_configs[]` inside `src/tasks.c`. Priorities and stack depths live in `include/system_defs.h`.

| Concern | Where it lives |
|---------|----------------|
| Task function body | `src/<module>.c` (or new file) |
| Task entry in table | `src/tasks.c` → `task_configs[]` |
| Priority / stack size | `include/system_defs.h` |
| Public task prototype | `include/<module>.h` or `include/tasks.h` |

Kernel tasks (Idle, TmrSvc) are created by FreeRTOS. DEBUG builds also register them with the telemetry task.

## IPC pattern

Tasks communicate through bounded FreeRTOS queues and task notifications — no shared mutable globals except `gSystemState` (written only by the state-machine task).

| Queue / mechanism | Producer(s) | Consumer | Payload |
|-------------------|-------------|----------|---------|
| `xFaultQueue` | FaultInject, manual `f` command | MemScrub | `FaultEvent_t` |
| `xStateRequestQueue` | Watchdog, MemScrub, MPU, commands | StateMachine | `StateRequest_t` |
| `xCommandQueue` | CommandInput | CommandHandler | `CommandType_t` |
| Watchdog notify bits | Heartbeat, MemScrub, FaultInject | Watchdog | `uint32_t` bits |

Init order in `main.c`: fault queue → command queue → memory protection → tasks.

## System states and SAFE policy

States are monotonic by default: **BOOT → NOMINAL → DEGRADED → SAFE**. Recovery to NOMINAL is allowed from DEGRADED or SAFE with reason `0x03` (watchdog) or `0x10` (operator UART `n`).

When the system enters **SAFE**, `safe_policy_on_enter()` in `src/safe_policy.c` applies a minimal-survival profile:

| Capability | NOMINAL / DEGRADED | SAFE |
|------------|-------------------|------|
| Automatic fault injection | On (toggle with `x`) | Forced off |
| Manual fault inject (`f`) | Allowed | Blocked |
| Background full-array scrub | Every idle MemScrub cycle | Skipped |
| Targeted scrub (fault queue) | Yes | Yes |
| Heartbeat UART line | Printed each second | Suppressed (watchdog kick continues) |
| FaultInject watchdog kick | Yes | No — watchdog expects only Heartbeat + MemScrub |
| SEU count → DEGRADED escalation | Yes | No (already at SAFE) |

Policy helpers are pure functions of `SystemState_t` where possible so they can be unit-tested on the host. Tasks call `system_state_get()` each cycle rather than caching state.

## Logging

UART output uses fixed prefixes from `include/log.h` (`[STATE]`, `[CMD]`, `[ERROR]`, `[SAFE]`, etc.). Keep new messages consistent so CI QEMU scripts and operators can grep logs reliably.

## Build layout

| Path | Contents |
|------|----------|
| `build/release/` | Object files for `make` |
| `build/debug/` | Object files for `make debug` (`-DDEBUG`) |
| `dist/release/` | Linked `kernel.elf`, `.bin`, `.map`, `size.txt`, `memory_report.txt` |
| `dist/debug/` | Same layout for DEBUG builds |

Release and debug builds never share object files. Run `make clean` before switching flavors if you are unsure of tree state.

## How to add a new task

Follow this checklist so the new task fits watchdog, telemetry, and CI expectations.

### 1. Define resources in `system_defs.h`

```c
#define TASK_PRIO_MYWORKER     1   /* must be 0 .. configMAX_PRIORITIES-1 */
#define TASK_STACK_MYWORKER    (configMINIMAL_STACK_SIZE * 2)
```

Pick a priority below the watchdog (4) and state machine (3) unless the task is safety-critical. Size stacks in **words** (×8 bytes on rv64); use DEBUG telemetry high-water marks to validate.

### 2. Implement the task function

Create `src/my_worker.c` and `include/my_worker.h`:

```c
void vTaskMyWorker(void *pvParameters)
{
    (void) pvParameters;

    for (;;) {
        /* do work */
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
```

If the task must survive SAFE mode, consult `include/safe_policy.h` and gate non-essential work with `safe_policy_*()` helpers. If the watchdog should monitor it, call `watchdog_kick(MY_BIT)` on every successful period and add the bit to `WATCHDOG_EXPECTED_BITS` (and update SAFE expectations if it should keep running in SAFE).

### 3. Register in `task_configs[]`

In `src/tasks.c`:

```c
#include "my_worker.h"

static const task_config_t task_configs[] = {
    /* ... existing entries ... */
    { "MyWorker", vTaskMyWorker, TASK_STACK_MYWORKER, TASK_PRIO_MYWORKER },
};
```

Order in the table does not set priority — `UBaseType_t priority` does — but keep related tasks grouped for readability.

### 4. Add the source file to the Makefile

Append to `CSRC` in the root `Makefile`:

```makefile
CSRC = src/main.c ... src/my_worker.c \
```

### 5. Initialize any IPC in `main` or a module `*_init()`

- Queues: create in `*_init()` called from `main()` before `tasks_create_all()`.
- Do not create queues inside the task body — boot checks assume init completed before the scheduler runs.

### 6. Expose operator commands (optional)

If the task should be UART-driven:

1. Add `CMD_MY_ACTION` to `CommandType_t` in `include/command.h`.
2. Map a key in `command_dispatch_char()` and handle it in `command_handle()` in `src/command.c`.
3. Prefer enqueueing to `xCommandQueue` from dispatch rather than doing heavy work in the input task.
4. Add host tests under `test/test_command.c` using the UART capture stub.

### 7. Add host tests

Pure logic → new `test/test_my_worker.c` + entry in `test/test_runner.c` and `TEST_CASE_OBJ` in the `Makefile`. Reuse stubs in `test/support/` for FreeRTOS and UART.

### 8. Update docs and CI expectations

- Mention the task in `README.md` (application tasks table).
- If the task prints boot-visible output, extend `scripts/qemu_smoke.sh` with a `require_pattern` line.

## Static analysis and tests

- `make test` — Unity host tests (no cross-compiler).
- `make lint` — cppcheck on `src/`.
- `make check` — both.

Firmware integration: `make qemu-smoke`, `make qemu-soak`.

## Linker and memory

`linker.ld` loads at `0x80000000` with separate ELF PHDRs for RX (`.text`/`.rodata`) and RW (`.data`/`.bss`). Critical watchdog code lives in `.text.critical` and is registered with the software MPU at boot.

`scripts/report_memory_map.sh` summarizes flash/RAM use from `kernel.elf` and `kernel.map`; CI runs it after each link.