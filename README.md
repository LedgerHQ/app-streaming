## Avantages

For users:

- plus de limite de nombre d'app à installer
- mise à jour automatique
- plus de changement de contexte de transport (déconnexion USB) lors du passage d'une app à une autre

For app developers:

- développement d'app facile
- plus de limite de taille de stack et de code
- ajout d'un heap et de malloc
- ne plus avoir io_exchange et d'exceptions (mécanisme clair pour recevoir des APDU et les boutons)
- système d'UX correct
- utilisation de librairies standard
- toolchain standard, utilisation de Rust de façon transparente
- plus de format d'APDU, protobuf ou JSON
- ne plus dépendre des évolutions du firmware
- mêmes applications pour tous les devices
- plus d'os_lib_call


## Limitations

- overhead (especially for BLE?)
- a companion  is always required (Ledger Live Desktop / Mobile)


## TODO

- malloc: newlib _sbrk
- sign code, encrypt writeable data
- syscalls: access to VM memory
- RISC-V: use compressed instructions
