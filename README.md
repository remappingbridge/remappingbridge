# RemappingBridge

Firmware plug-and-play para Raspberry Pi Pico 2 W que recebe Mouse HID Bluetooth e expõe uma identidade USB HID estável ao host, sem software no sistema operacional.

## Versão estável

**RemappingBridge 2.0.0** — Movimento horizontal/vertical independente, rotação anticlockwise de 0° a 270°, persistência de DPI e rotação por Mouse Bluetooth, Saved Devices por identidade e Help contextual. Todos os gates **MOV-01 a MOV-10** foram aceitos fisicamente.

A versão mantém pareamento/reconexão HID Bluetooth, remoção de dispositivos, remapeamento Passthrough/Standard/Escape/Custom, Logitech Lift HID++, tela de bloqueio, interface 9×21 e identidade USB HID da v1.0.0.

Firmware de produção e checksums: [Release v2.0.0](https://github.com/remappingbridge/remappingbridge/releases/tag/v2.0.0).
Novidades e instruções: [docs/releases/v2.0.0.md](docs/releases/v2.0.0.md).

## Fontes normativas

Para a versão 2.0, a precedência documental é:

1. [Movement Options — planner](https://github.com/remappingbridge/planner/tree/main/movement-options) e seus gates aceitos, para as telas e comportamentos novos/alterados;
2. `docs/releases/v2.0.0.md` — contrato do lançamento;
3. `docs/hope/00-final-screen-inventory.md`, `02-final-navigation.md` e `01-final-architecture.md` — comportamento anterior da v1.0.0 mantido quando não alterado pela v2.0;
4. documentos Gxx/pré-HOPE e `docs/reference/00-reuse-evidence.md` — histórico técnico, não autoridade sobre a UX atual.

Em caso de conflito, não se interpreta silenciosamente: a implementação para e o contrato é corrigido antes do código.

## Regra de layout

Os layouts documentados são congelados. Texto literal não pode ser reformulado por conveniência. Somente conteúdo explicitamente marcado como exemplo/dinâmico pode ser proposto pela implementação, respeitando a grade 9x21, as regiões visuais e as regras semânticas de cor.

## Regra de depuração

O firmware deste repositório **não terá ferramentas de depuração embarcadas**: sem USB CDC de diagnóstico, UART de diagnóstico, descriptor USB alternativo de debug, tela de debug ou firmware paralelo de debug.

Diagnóstico deve ocorrer por testes host/CI, revisão de estado, critérios físicos no LCD/HAT e comportamento observável pelo sistema operacional.

## Gates

O planejamento congelado está em `plan/gates.md` e o checklist contínuo em `plan/implementation-checklist.md`.
