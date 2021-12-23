## Avantages

For users:

- plus de limite de nombre d'app à installer
- mise à jour automatique
- plus de changement de contexte de transport (déconnexion USB) lors du passage d'une app à une autre

For app developers:

- apps are maintainable without any specific knowledge
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
- testing is made easy


## Limitations

- overhead (especially for BLE?). Performances could be increased by removing the IO task
- a companion  is always required (Ledger Live Desktop / Mobile)


## How to build a RISC-V app

TODO: create a Dockerfile

`docker pull dockcross/linux-riscv32:latest` + newlib


## TODO

- sign code, encrypt writeable data
- syscalls: access to VM memory
- RISC-V: use compressed instructions
