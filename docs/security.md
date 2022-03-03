Everything is encrypted: the code of the app, data, stack and heap.

Each page is made of 256 bytes of data associated to an address (4 bytes). A page has an unique counter (4 bytes).

All pages exchanged between the host and the VM are:

- encrypted using AES-256. The page data (256 bytes) is encrypted with the key `KeyAES`. The IV is `addr || counter || '\x00' * 8` where `addr` and `counter` are 4 bytes each, encoded in little-endian.
- authenticated using HMAC-SHA256. The following message is authenticated using the key `KeyHMAC`: `encrypted_data || addr || counter` where `encrypted_data` is the page data (256 bytes) encrypted using AES followed by `addr` and `counter` which are 4 bytes each and encoded in little-endian.

There are 2 sets of 2 keys for AES encryption/decryption and HMAC authentication:

1. A static set (`KeyAES1`, `KeyHMAC1`): shared between the host and the VM.
2. A dynamic set (`KeyAES2`, `KeyHMAC2`): initialized randomly each time an app is launched.


## Behavior

There are 2 kind of exchanges, always initiated by the VM:

- Requesting a page, given its address. The page data, counter, HMAC and merkle tree proof are returned by the host.
- Committing a writeable page. The page data, counter and HMAC are sent by the VM.

Committing a page always increments its associated counter.


## Different kind of pages

### Code and read-only data

Code and read-only data pages are encrypted and authenticated using the static key set. The counter of these pages is always `0` and can never change.

### Initialized data

Initialized data pages are encrypted and authenticated using the static key set. The counter of these pages is initialized to `0` and is incremented over time when the VM modifies these pages. The VM will then use the dynamic key set.

### Stack and heap

Stack and heap pages don't exist initially. They're created by the VM and encrypted and authenticated using the dynamic key set. The counter is initialized to `1`.


## Anti-replay

Thanks to encryption, an attacker has no knowledge over the content of pages. Thanks to authentication, an attacker can't forge invalid pages. However an attacker could replay valid pages that where later modified by the VM.

Each writeable page is part of a merkle tree whose root hash is kept by the VM. The merkle tree is initialized with the writeable pages from the app. Each time the VM commit a page, this page is either inserted in the merkle tree (for a new stack page or heap page) or the corresponding node in the merkle tree is updated.

The nodes of these merkle tree are made of 8 bytes of data associated to a page: `addr || counter`. It's guaranteed that there's a unique node in the tree associated to an address.

When the VM receives a writeable page from the host, the merkle tree is used to ensure that the node made of the associated address and counter is actually part of the tree. It guarantees that the counter associated to the page is valid.


## Key management (to be discussed)

Once an app is built, pages are encrypted and authenticated using `KeyAES1` and `KeyHMAC1`. Several options are available:

- A unique key could be shared by all apps and all Nanos.
- A key derived for each app could be shared by all Nanos.
- A key derived for each app and each Nano could be generated depending on the Nano ID when the user retrieve an app.

If an attacker manages to compromise BOLOS, he might retrieve the key used to authenticate app pages (`KeyHMAC1`) and create malicious apps. In order to prevent that scenario a new authentication key should be generated for **each** Nano and **each** app.

1. The host (Ledger Live or Python client) asks the Nano an ECDSA public key `pub1`, generated and signed (`sig1`) by BOLOS.
2. `pub1` and `sig1` are sent to a Ledger HSM which verifies the signature.
3. This Ledger HSM:
    - Generates a new and random `KeyAES1` and `KeyHMAC1`
    - Authenticates the app pages using `KeyHMAC1`
    - Encrypts a manifesto which contains `KeyAES1` and `KeyHMAC1` using `pub1`
    - Sends the encrypted manifesto and the app to the host
