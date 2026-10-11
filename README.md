# RemappingBridge

Firmware plug-and-play para **Raspberry Pi Pico 2 W**: recebe mouse HID Bluetooth e expõe dispositivos USB HID de Mouse e Keyboard/Consumer a hosts, sem instalar software específico.

## Versão estável

**RemappingBridge 3.0.1** — primeira manutenção PATCH sobre a v3.0.0, com o **MNT-01 aceito fisicamente pelo operador**. A CI do candidato MNT-01 passou em **81/81 testes automatizados**, e o commit de release deverá repetir essa qualificação. Os gates MOD-00 a MOD-10 da v3.0.0 permanecem aceitos. Os aceites gerais não são um registro de PASS individual para cada plataforma/aplicativo.

### Modifier Shortcuts
- Dois perfis: **PASSTHROUGH** e **CUSTOM**. As opções antigas STANDARD REMAP e ESCAPE REMAP foram removidas da UI.
- Um dos cinco botões físicos (Left/Right/Middle/Forward/Backward) pode ser configurado como **MODIFIER**, sem gerar seu clique físico durante a combinação.
- **12 combinações**: MODIFIER RELEASED, cinco botões, WHEEL UP/DOWN e MOVE LEFT/RIGHT/UP/DOWN.
- **30 ações** de teclado e mídia, com fluxos HOME CONNECTED → REMAPPING OPTIONS → MODIFIER SHORTCUTS → COMBINATIONS 1..6 → SHORTCUTS 1..5 → KEY A: APPLY AND BACK; reset por seleção.
- Persistência global de profile e assignments no schema v3; `UNAVAILABLE` quando uma combinação usa o próprio Modifier, sem apagar a atribuição.
- Proteção contra stuck-key, interrupção BLE/USB, filas sob backpressure e cancelamento em lock/reconfiguração.

### Funcionalidades preservadas da v2
- **Movement Options**: ganho horizontal e vertical independentes, rotação 0°/90°/180°/270° e Movement per Mouse Bluetooth.
- Reconexão/pareamento BLE, Saved Devices, Logitech Lift HID++, Help/lock, tela 9×21, wheel/pan e identidade USB estável.
- Na tela **Saved Devices**, o cabeçalho usa magenta padrão, o nome do mouse ativo é o único texto ciano, os dados do mouse permanecem amarelos, **REMOVE DEVICE** é branco quando acionável e as dicas são cinzas.

Firmware/checksums: [Release v3.0.1](https://github.com/remappingbridge/remappingbridge/releases/tag/v3.0.1) (publicação após CI final). Detalhes: [docs/releases/v3.0.1.md](docs/releases/v3.0.1.md). Histórico principal: [v3.0.0](docs/releases/v3.0.0.md).

## Compatibilidade de atualização

A atualização normal é **UF2 em modo BOOTSEL, sem apagar toda a flash**. O firmware converte perfil global legado v2 para v3; bonds BLE e Movement por Mouse pertencem a outros registros e não devem ser apagados na atualização. O downgrade de v3 para firmware v2.0.0 **não é totalmente compatível**: o v2 não lê o schema v3 e pode restaurar o perfil global aos valores padrão. Anote os 12 slots e o perfil antes de qualquer downgrade; não faça `flash_nuke` para atualizar.

Os efeitos de alguns atalhos são dependentes do host. Em particular, **zoom no Obsidian/KDE**, atalhos Android HOME e controle real de brilho devem ser verificados no sistema alvo. Um código HID válido não garante comportamento do aplicativo.

## Fontes normativas

1. [Modifier Shortcuts — planner, gates MOD-00..MOD-10](https://github.com/remappingbridge/planner/tree/main/modifier-shortcuts), contrato aprovado da v3.
2. [Manutenção v3.x / MNT-01](https://github.com/remappingbridge/planner/tree/main/v3-maintenance) e [notas v3.0.1](docs/releases/v3.0.1.md); [contrato v3.0.0](docs/releases/v3.0.0.md) como baseline.
3. [Movement Options — planner](https://github.com/remappingbridge/planner/tree/main/movement-options), funcionalidades herdadas da v2.
4. [Release v2.0.0](docs/releases/v2.0.0.md) e documentação de telas anteriores, quando não revogadas pela v3.

Em conflito de UI não alterar textos/telas congeladas unilateralmente; usar a grade 9×21 e as cores do design system.

## Regra de depuração

Sem ferramentas de debug embarcadas: não fornecer USB CDC/UART de diagnóstico, tela de debug nem um firmware debug paralelo. Diagnóstico via testes host/CI, estado observável na interface e comportamento do SO.
