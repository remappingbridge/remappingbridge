# RemappingBridge

Firmware plug-and-play para Raspberry Pi Pico 2 W que recebe Mouse HID Bluetooth e expõe uma identidade USB HID estável ao host, sem software no sistema operacional.

## Versão estável

**RemappingBridge 1.0.0** é a primeira versão estável concluída após a série HOPE.

O firmware final inclui pareamento/reconexão de Mouse, Saved Devices persistentes, remoção de dispositivos, perfis Passthrough/Standard/Escape/Custom, Logitech Lift HID++, Lock/Help e a UX final de 29 telas canônicas.

## Fontes normativas

Para a versão 1.0, a precedência documental é:

1. `docs/hope/00-final-screen-inventory.md` — inventário final das 29 telas;
2. `docs/hope/02-final-navigation.md` — navegação e regras de controle;
3. `docs/hope/01-final-architecture.md` — arquitetura efetivamente implementada;
4. documentos Gxx/pré-HOPE — histórico e evidência, somente quando não conflitarem com `docs/hope/`;
5. `docs/reference/00-reuse-evidence.md` — conhecimento reaproveitável, nunca autoridade sobre a UX final.

Em caso de conflito, não se interpreta silenciosamente: a implementação para e o contrato é corrigido antes do código.

## Regra de layout

Os layouts documentados são congelados. Texto literal não pode ser reformulado por conveniência. Somente conteúdo explicitamente marcado como exemplo/dinâmico pode ser proposto pela implementação, respeitando a grade 9x21, as regiões visuais e as regras semânticas de cor.

## Regra de depuração

O firmware deste repositório **não terá ferramentas de depuração embarcadas**: sem USB CDC de diagnóstico, UART de diagnóstico, descriptor USB alternativo de debug, tela de debug ou firmware paralelo de debug.

Diagnóstico deve ocorrer por testes host/CI, revisão de estado, critérios físicos no LCD/HAT e comportamento observável pelo sistema operacional.

## Gates

O planejamento congelado está em `plan/gates.md` e o checklist contínuo em `plan/implementation-checklist.md`.
