#include "gui.h"

int InvadersGUI::setup_debugger_gui() {
#if defined(__APPLE__)
    // GL 3.2 Core + GLSL 150
    const char* glsl_version = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    float main_scale = ImGui_ImplSDL2_GetContentScaleForDisplay(0);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    m_debugger_window = SDL_CreateWindow("i8080 Emulator Debugger", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, (int)(810 * main_scale), (int)(800 * main_scale), window_flags);
    if (m_debugger_window == nullptr) {
        fprintf(stderr, "Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return 1;
    }

    m_gl_context = SDL_GL_CreateContext(m_debugger_window);
    if (m_gl_context == nullptr) {
        fprintf(stderr, "Error: SDL_GL_CreateContext(): %s\n", SDL_GetError());
        return 1;
    }

    SDL_GL_MakeCurrent(m_debugger_window, m_gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(m_debugger_window, m_gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    return 0;
}

int InvadersGUI::setup() {

#ifdef _WIN32
    ::SetProcessDPIAware();
#endif

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
      fprintf(stderr, "Error: %s\n", SDL_GetError());
      return 1;
    }

    m_game_window = SDL_CreateWindow("Space Invaders", 0, 0, 224, 256, SDL_WINDOW_SHOWN);
    if (!m_game_window) {
      fprintf(stderr, "Could not create SDL window: %s\n", SDL_GetError());
      return 1;
    }
    m_game_renderer = SDL_CreateRenderer(m_game_window, -1, 0);
    SDL_RenderPresent(m_game_renderer);

#ifndef NDEBUG
    if(setup_debugger_gui()) { 
        return 1;
    }
#endif

    return 0;
}

int InvadersGUI::update_game_window(const MemoryState& memory_state) {

#ifndef NDEBUG
    SDL_GL_MakeCurrent(m_game_window, nullptr);
#endif

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            return 1;
        }
        if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(m_game_window)) {
            return 1;
        }
    }

    SDL_SetRenderDrawColor(m_game_renderer, 0, 0, 0, 0);
    SDL_RenderClear(m_game_renderer);
    for (size_t i = 1; i < 224; i++) {
        int r = 0;
        for(size_t j = 0; j < 32; j++){
            uint8_t byte = memory_state.ram_state[0x0400 + (i*0x20) - j];
            for(int k = 7; k >= 0; k--, r++) {
                uint8_t colour = !!(byte & (1 << k)) ? 255 : 0;
                SDL_SetRenderDrawColor(m_game_renderer, colour, colour, colour, colour);
                SDL_RenderDrawPoint(m_game_renderer, i, r);
            }
        }
    }
    SDL_RenderPresent(m_game_renderer);

    return 0;
}

#ifndef NDEBUG
int InvadersGUI::update_debugger_window(const CpuState& cpu_state, const MemoryState& memory_state, 
                                        unsigned int breakpoints[], size_t breakpoints_size,
                                        bool& debugger_step, bool& debugger_cpu_running)
{
    SDL_GL_MakeCurrent(m_debugger_window, m_gl_context);

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    ImGuiIO& io = ImGui::GetIO(); 

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);

        if (event.type == SDL_QUIT) {
            return 1;
        }
        if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(m_debugger_window)) {
            return 1;
        }
        // leaving this here so i can close the game window even when its not updating (due to the cpu being paused for example)
        if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(m_game_window)) {
            return 1;
        }
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    const ImGuiViewport* viewport = ImGui::GetMainViewport();

    {
        ImGui::SetNextWindowPos(ImVec2(viewport->WorkSize.x * 0.5f, viewport->WorkPos.y));
        ImGui::SetNextWindowSize(viewport->WorkSize);

        static MemoryEditor mem_edit;
        mem_edit.OptShowAscii = false;
        mem_edit.ReadOnly = true;
        mem_edit.DrawWindow("Intel 8080 - Memory Editor", memory_state.ram_state, INVADERS_RAM_SIZE, 0x2000);
    }

    {
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x * 0.5f, viewport->WorkSize.y));

        ImGui::Begin("Intel 8080 - Debugger", nullptr, ImGuiWindowFlags_NoCollapse);

        static bool follow_execution = true;
        ImGui::Checkbox("Follow Execution", &follow_execution);

        if(ImGui::Button("CPU Step")) { debugger_step = true; }

        ImGui::SameLine();

        char run_pause[16];
        if(!debugger_cpu_running){
            strcpy(run_pause, "Run");
        } else { 
            strcpy(run_pause, "Pause");
        }

        if(ImGui::Button(run_pause)) { 
            debugger_cpu_running = !debugger_cpu_running;
        }

        ImGui::SetNextItemWidth(50.0f);

        static bool breakpoints_initialised = false;
        static size_t available_breakpoint_slot_idx = 0;
        for(size_t i = 0; i < breakpoints_size && !breakpoints_initialised; i++) {
            breakpoints[i] = 0xf0000;
        }
        breakpoints_initialised = true;

        static char breakpoint_addr_str[5];
        unsigned int breakpoint_addr {};
        if (ImGui::InputText("##addr", breakpoint_addr_str, 5, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_EnterReturnsTrue)) {
            if(sscanf(breakpoint_addr_str, "%04x", &breakpoint_addr)){
                while(available_breakpoint_slot_idx < breakpoints_size && breakpoints[available_breakpoint_slot_idx] != 0xf0000) {
                    available_breakpoint_slot_idx++;
                }
                if(available_breakpoint_slot_idx >= breakpoints_size) {
                    printf("exceeded max number of breakpoints %lu, can't add anymore\n", breakpoints_size);
                } else {
                    breakpoints[available_breakpoint_slot_idx] = breakpoint_addr;
                    available_breakpoint_slot_idx++;
                }
                strcpy(breakpoint_addr_str, "");
            }
        }

        ImGui::SameLine();
        ImGui::Text("Breakpoint Address");

        const uint8_t* instructions = memory_state.rom_state;
        size_t instructions_size = INVADERS_ROM_SIZE;

        if (ImGui::BeginTable("##Instructions", 4, ImGuiTableFlags_ScrollY, ImVec2(0.0f, ImGui::GetFontSize() * 45))) {

            ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthFixed, 50.0f);
            ImGui::TableSetupColumn("Instruction Binary", ImGuiTableColumnFlags_WidthFixed, 150.0f);
            ImGui::TableSetupColumn("Instruction", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            ImGui::TableSetupColumn("Breakpoint Active", ImGuiTableColumnFlags_WidthFixed, 100.0f);

            // TODO: get this shit outta here man, why am i disassembling everytime lol
            DisassembledInstruction disassembled_instructions[INVADERS_ROM_SIZE] {};
            size_t disassembled_rom_size = disassemble_rom(disassembled_instructions, INVADERS_ROM_SIZE, instructions, instructions_size);

            size_t i,j;
            for(i=0, j=0; i < disassembled_rom_size; i++) {
              ImGui::TableNextRow();

              // Address
              ImGui::TableNextColumn();
              ImGui::PushID(j);

              char buf[32];
              sprintf(buf, "0x%04x: ", (unsigned int)j);
              ImGui::Text(buf);

              // Instruction Binary
              ImGui::TableNextColumn();

              ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
              switch(disassembled_instructions[i].byte_count){
                case 1: sprintf(buf, "%02x", instructions[j]); break;
                case 2: sprintf(buf, "%02x %02x", instructions[j], instructions[j+1]); break;
                case 3: sprintf(buf, "%02x %02x %02x", instructions[j], instructions[j+1], instructions[j+2]); break;
                default:
                    strcpy(buf, "???"); break;
              }
              ImGui::Text(buf);
              ImGui::PopStyleColor(); 

              // Instruction
              ImGui::TableNextColumn();

              ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.0f, 1.0f));
              ImGui::Selectable(disassembled_instructions[i].ins_str, j == cpu_state.pc, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap);
              if(j == cpu_state.pc && follow_execution) { 
                  ImGui::SetScrollHereY(); 
              }

              ImGui::PopID();
              ImGui::PopStyleColor(); 

              ImGui::TableNextColumn();
              ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
              for(size_t k = 0; k < breakpoints_size; k++) {
                  if(breakpoints[k] == j){
                      ImGui::Text("*");
                      break;
                  }
              }
              ImGui::PopStyleColor(); 

              j += disassembled_instructions[i].byte_count;
            }
            ImGui::EndTable();

            ImGui::Text("CPU State:");

            char buf[128];
            sprintf(buf, "PC: 0x%04x SP: 0x%04x A: 0x%02x", (unsigned int)cpu_state.pc, cpu_state.rps.sp, cpu_state.rgs.a);
            ImGui::Text(buf);

            sprintf(buf, "B: 0x%02x C: 0x%02x D: 0x%02x E: 0x%02x H: 0x%02x L: 0x%02x", 
                        cpu_state.rgs.b, cpu_state.rgs.c, cpu_state.rgs.d, cpu_state.rgs.e, cpu_state.rgs.h, cpu_state.rgs.l);
            ImGui::Text(buf);

            sprintf(buf, "BC: 0x%02x DE: 0x%02x HL: 0x%02x", cpu_state.rps.bc, cpu_state.rps.de, cpu_state.rps.hl);
            ImGui::Text(buf);

            sprintf(buf, "Z: %d | S: %d | P: %d | CY: %d | AC: %d", cpu_state.rps.psw.z , cpu_state.rps.psw.s , cpu_state.rps.psw.p, 
                                                                    cpu_state.rps.psw.cy, cpu_state.rps.psw.ac);
            ImGui::Text(buf);
        }

        ImGui::End();
    }

    // keeping this to figure out shit when i need to add more gui thingies
    // ImGui::ShowDemoWindow();

    ImGui::Render();
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);

    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(m_debugger_window);

    return 0;
}
#else 
int InvadersGUI::update_debugger_window(const CpuState&, const MemoryState&, unsigned int[], size_t, bool&, bool&) {
    fprintf(stderr, "function \e[1m'%s()'\e[0m only works in debug mode.\n", __FUNCTION__);
    return 1;
}
#endif /* ifndef NDEBUG */ 

void InvadersGUI::destroy() {
#ifndef NDEBUG
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(m_gl_context);
    SDL_DestroyWindow(m_debugger_window);
#endif
    SDL_DestroyWindow(m_game_window);
    SDL_DestroyRenderer(m_game_renderer);
    SDL_Quit();
}
