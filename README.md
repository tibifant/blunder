# Blunder

## A Simple Chess Engine written in C++

- Minimax-Algorithm with Alpha-Beta-Pruning
- Quiescence Search
- Move Ordering (MVV/LVA)
- Iterative Deepening with Aspiration Windows
- HTTP API via crow with sample 3D-Web-UI
- Command Line based UI

<a><img src="./images/conio.png" alt="console application" style="width: pt; max-width: 100%; margin: 10pt auto;"></a>
<br>
<a><img src="./images/webio.png" alt="web application" style="width: 533pt; max-width: 100%; margin: 10pt auto;"></a>
<br>

### How to build the project:
```bat
create_project.bat
MSBuild /p:Configuration=Release /nologo /v:m
```
