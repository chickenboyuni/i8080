#include "gui.h"

int InvadersGUI::setup() {

#ifdef _WIN32
    ::SetProcessDPIAware();
#endif

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
        printf("Error: %s\n", SDL_GetError());
        return 1;
    }

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
    m_window = SDL_CreateWindow("i8080 Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, (int)(1280 * main_scale), (int)(800 * main_scale), window_flags);
    if (m_window == nullptr) {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return 1;
    }

    m_gl_context = SDL_GL_CreateContext(m_window);
    if (m_gl_context == nullptr) {
        printf("Error: SDL_GL_CreateContext(): %s\n", SDL_GetError());
        return 1;
    }

    SDL_GL_MakeCurrent(m_window, m_gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(m_window, m_gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    return 0;
}

int InvadersGUI::update_frame(const CpuState& cpu_state, bool& debugger_step, bool& debugger_cpu_running, const uint8_t instructions[], size_t instructions_size) {

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    ImGuiIO& io = ImGui::GetIO(); 

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);

        if (event.type == SDL_QUIT) {
            return 1;
        }
        if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(m_window)) {
            return 1;
        }
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();


    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

#ifndef NDEBUG
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

    if (ImGui::BeginTable("##Instructions", 3, ImGuiTableFlags_ScrollY, ImVec2(0.0f, ImGui::GetFontSize() * 50))) {

        ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthFixed, 50.0f);
        ImGui::TableSetupColumn("Instruction Binary", ImGuiTableColumnFlags_WidthFixed, 150.0f);
        ImGui::TableSetupColumn("Instruction", ImGuiTableColumnFlags_WidthFixed, 100.0f);

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
          switch(disassembled_instructions[i].size){
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
          ImGui::Selectable(disassembled_instructions[i].ins.c_str(), j == cpu_state.pc, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap);
          if(j == cpu_state.pc && follow_execution) { 
              ImGui::SetScrollHereY(); 
          }

          j += disassembled_instructions[i].size;

          ImGui::PopID();

          ImGui::PopStyleColor(); 
        }
        ImGui::EndTable();

        ImGui::Text("CPU State:");

        char buf[128];
        sprintf(buf, "PC: 0x%04x A: 0x%02x", (unsigned int)cpu_state.pc, cpu_state.rgs.a);
        ImGui::Text(buf);

        sprintf(buf, "B: 0x%02x C: 0x%02x D: 0x%02x E: 0x%02x H: 0x%02x L: 0x%02x", 
                    cpu_state.rgs.b, cpu_state.rgs.c, cpu_state.rgs.d, cpu_state.rgs.e, cpu_state.rgs.h, cpu_state.rgs.l);
        ImGui::Text(buf);

        sprintf(buf, "BC: 0x%02x DE: 0x%02x HL: 0x%02x", cpu_state.rps.bc, cpu_state.rps.de, cpu_state.rps.hl);
        ImGui::Text(buf);
    }

    ImGui::End();
#endif /* ifndef NDEBUG */

    // keeping this to figure out shit when i need to add more gui thingies
    // ImGui::ShowDemoWindow();

    ImGui::Render();
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);

    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(m_window);

    return 0;
}

void InvadersGUI::destroy() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(m_gl_context);
    SDL_DestroyWindow(m_window);
    SDL_Quit();
}
