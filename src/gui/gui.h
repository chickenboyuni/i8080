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

class InvadersGUI {
  public:
    InvadersGUI() = default;
    ~InvadersGUI() = default;

    int setup();
    int update_game_window(const MemoryState& memory_state);
    int update_debugger_window(const CpuState& cpu_state, const MemoryState& memory_state, 
                     unsigned int breakpoints[], size_t breakpoints_size,
                     bool& debugger_step, bool& debugger_cpu_running);
    void destroy();

  private:
    SDL_Window* m_debugger_window = nullptr;
    SDL_GLContext m_gl_context = nullptr;

    SDL_Window* m_game_window = nullptr;
    SDL_Renderer* m_game_renderer = nullptr;

    int setup_debugger_gui();
};

int display_gui();


#endif /* GUI_H */
