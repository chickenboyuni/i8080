#ifndef GUI_H
#define GUI_H

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include "imgui_memory_editor.h"

#include "../disasm/disasm.h"
#include "../core/invaders/invaders.h"
#include "../core/cpu.h"

#include <stdio.h>
#include <SDL.h>
#include <SDL_opengl.h>

#ifdef _WIN32
#include <windows.h>
#endif

enum INP1_INPUTS {
    INP1_CREDIT = 1 << 0,
    INP1_P2_START = 1 << 1,
    INP1_P1_START = 1 << 2,
    INP1_P1_SHOT = 1 << 4,
    INP1_P1_LEFT = 1 << 5,
    INP1_P1_RIGHT = 1 << 6,
};

enum INP2_INPUTS {
    INP2_P2_SHOT = 1 << 4,
    INP2_P2_LEFT = 1 << 5,
    INP2_P2_RIGHT = 1 << 6,
};

typedef struct InvadersInputs {
    uint8_t inp0;
    uint8_t inp1;
    uint8_t inp2;
} InvadersInputs;

class InvadersGUI {
  public:
    InvadersGUI() = default;
    ~InvadersGUI() = default;

    InvadersInputs get_inputs() { return m_inputs; }

    int setup();
    int update_game_window(const MemoryState& memory_state);
    int update_debugger_window(const CpuState& cpu_state, const MemoryState& memory_state, 
                     unsigned int breakpoints[], size_t breakpoints_size,
                     bool& debugger_step, bool& debugger_cpu_running);
    void destroy();

  private:
    InvadersInputs m_inputs {.inp0 = 0b00001110, .inp1 = 0b00001000, .inp2 = 0b00000000};

    SDL_Window* m_debugger_window = nullptr;
    SDL_GLContext m_gl_context = nullptr;

    SDL_Window* m_game_window = nullptr;
    SDL_Renderer* m_game_renderer = nullptr;

    int setup_debugger_gui();
};

#endif /* GUI_H */
