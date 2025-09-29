# Blunder

## A Chess Engine written in C/C++

- Minimax-Algorithm with Alpha-Beta-Pruning
- Quiescence Search
- Move Ordering (MVV/LVA)
- Iterative Deepening with Aspiration Windows

Playable as **Console-Application** or via **3D-Web-UI**

## How to build the project:
- Run `create_project.bat`
- Open the solution (`blunder.sln`) in Visual Studio 2022
- Build the solution (Build > Build Solution)

## How to run the server:
- Execute `builds/bin/webioD.exe` (Debug) / `builds/bin/webio.exe` (Release) 
- Queries will be processed on port `21110` 
