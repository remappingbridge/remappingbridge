# MOD-01 — Remapping Options (candidato de teste físico)

Baseline: tag imutável `v2.0.0`. Branch de gate: `feature/modifier-shortcuts-mod01`.
Planejamento: https://github.com/remappingbridge/planner/tree/main/modifier-shortcuts

## Escopo efetivo

- Menu Remapping Options com **PASSTHROUGH DEFAULT**, **CUSTOM REMAPPING**, **MODIFIER SHORTCUTS**.
- `JOY UP/DOWN` seleciona uma das três opções, `JOY PRESS` acessa as rotas disponíveis, `KEY B` volta para Home, `KEY X` exibe Help com os nomes novos.
- A rota Custom está na segunda linha. A terceira linha é reconhecida, mas seu `JOY PRESS` é **intencionalmente no-op até MOD-02**, que criará COMBINATIONS 1 OF 6; nenhuma tela provisória é adicionada.
- Somente Passthrough e Custom aparecem no resumo da HOME e em PROFILE de Saved Devices. Uma configuração v2 com perfil Standard/Escape ativo é lida, mas **normalizada para PASSTHROUGH somente em RAM** durante a inicialização. Este gate não atualiza o schema v2, não altera bonds e não escreve flash nessa transição. A migração persistente formal pertence ao MOD-03.
- **As quatro antigas telas** de confirmação Standard/Escape (APPLY + APPLIED) e seus comandos UX foram removidos do código e do inventário de telas neste MOD-01: 38 → **34 telas de runtime**. O destino `ESCAPE` do editor Custom WILL BECOME e os valores serializados de perfil legado são **mantidos temporariamente** para proteger a migração v2→v3, sendo substituídos no MOD-04/MOD-03 respectivamente.
- USB HID, BLE, Movement, cores Saved Devices e formato persistente da v2 mantidos neste gate.

## Teste físico (operador)
1. Atualize com o `.uf2` gerado pela CI desse commit **sem apagar flash e sem flash_nuke**. Conecte o mouse salvo até entrar em `home-connected`.
2. Com a primeira linha `NO REMAP PASSTHROUGH` selecionada, pressione e solte `JOY PRESS`. Confira exatamente as três opções, nesta ordem. STANDARD/ESCAPE não podem estar presentes.
3. No menu, `JOY UP` na primeira opção deve selecionar a terceira; `JOY DOWN` volta à primeira. Confira seleção branca e Passthrough atual em ciano.
4. `JOY PRESS` em PASSTHROUGH DEFAULT abre a confirmação/estado já aplicado; `KEY B` retorna às três opções.
5. Selecione CUSTOM REMAPPING, pressione `JOY PRESS`; confira o editor Custom normal. Sem trocar atribuições, `KEY B` deve voltar às três opções. Se Custom já estava ativo, seu nome aparece ciano no menu.
6. Selecione MODIFIER SHORTCUTS, pressione `JOY PRESS`; neste MOD-01 o menu deve **permanecer na mesma tela/posição**, sem entrar numa tela de Standard/Escape e sem produzir atalhos. O acesso a COMBINATIONS é do MOD-02.
7. Com a terceira selecionada, `KEY X` abre `REMAPPING OPTIONS HELP`, que menciona Passthrough Default, Custom Remapping e Modifier Shortcuts; qualquer tecla volta à mesma opção.
8. `KEY Y` bloqueia a tela (backlight desligado); um `JOY PRESS` libera o lock, **sem acionar** a opção. Retorne à Home com `KEY B`.
9. Navegue a SAVED DEVICES e confirme que PROFILE exibe PASSTHROUGH ou CUSTOM, nunca STANDARD/ESCAPE; apenas o nome do mouse conectado fica ciano. Os valores atuais DPI H/V e ROTATION devem permanecer.
10. Movimente o ponteiro, role e clique; valide rotação e ganhos H/V já configurados. Desconecte e reconecte o mouse; repita a abertura do menu. O mouse salvo deve manter bond/nome/Movement.

## Não são aceites deste gate
- Telas COMBINATIONS e SHORTCUTS, atribuição/persistência de shortcut, botão MODIFIER, gestos, mídia e alterações de descriptor HID.
- `v2.0.0` e `main` não são modificados por esta branch; pull request permanece em draft até aceite.
