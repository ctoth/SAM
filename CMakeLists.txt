cmake_minimum_required(VERSION 3.10)

project(Sam)

add_executable(Sam src/cli.c src/phoneme_parser.c src/reciter.c src/sam.c src/render.c src/rules.c src/sam_tabs.c src/transitions.c src/wav.c)