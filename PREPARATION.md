# HOPE preflight — no implementation yet

Status: **PREPARED / HOPE-00 NOT STARTED**.

Este arquivo existe somente na branch documental `planning/hope-preflight`.

## Produto

A `main` deve permanecer no commit inicial até o início explícito do HOPE-00.

O HOPE-00 futuro deve criar sua própria branch a partir da `main` limpa e transferir exatamente a baseline:

~~~text
remappingbridge/blu2usb
gate/g06-profiles-remap-logitech-hidpp
7eee024ad4ee726c5a85ffa2f32b9f47187878af
~~~

## UX posterior

Após HOPE-00 aceito, as telas serão substituídas uma por vez segundo:

~~~text
remappingbridge/mouse-ui
release/ui-layout-v1.0
e8adad7919e931c92515bf655ef4050876a8e7a9
~~~

Mouse UI é referência de comportamento visual/interação das telas, não arquitetura a ser importada.

## Planner normativo

~~~text
remappingbridge/repo-planner/hope
~~~

## Proibido nesta preparação

- copiar firmware;
- modificar C/CMake;
- produzir UF2;
- iniciar HOPE-00;
- importar contratos UI↔Core;
- importar mouse-core;
- habilitar Keyboard/Composite.

A branch `planning/hope-preflight` **não deve ser usada como base do firmware HOPE-00**.
