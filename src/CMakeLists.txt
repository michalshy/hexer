add_executable(hexer
    main.cpp
    app.cpp
    terminal.cpp
)

target_link_libraries(hexer PRIVATE hexer_core)
