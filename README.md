## Avantages

- plus de limite de taille de stack et de code
- ajout d'un heap et de malloc
- changement de contexte de transport (déconnexion USB)
- ne plus avoir io_exchange et d'exceptions (mécanisme clair pour recevoir des APDU et les boutons)
- système d'UX correct
- utilisation de librairies standard
- toolchain standard, utilisation de Rust de façon transparente
- plus de format d'APDU, protobuf ou JSON

- ne plus dépendre des évolutions du firmware

- développement d'app facile
- plus de limite de nombre d'app à installer
- mise à jour automatique
- mêmes applications pour tous les devices


## TODO

- malloc: newlib _sbrk
- sign code, encrypt writeable data
- syscalls: access to VM memory
- RISC-V: use uncompressed instructions
