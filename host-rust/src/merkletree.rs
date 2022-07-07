use crypto::digest::Digest as CryptoDigest;
use crypto::sha2::Sha256;

#[cfg(test)]
use hex_literal::hex;

type Digest = [u8; 32];

#[derive(Clone, Copy)]
struct Entry {
    addr: u32,
    counter: u32,
}

impl Entry {
    pub fn to_bytes(self) -> [u8; 8] {
        let n = ((self.counter as u64) << 32) | (self.addr as u64);
        n.to_le_bytes()
    }

    pub fn update_counter(&mut self, counter: u32) {
        self.counter = counter;
    }
}

struct MerkleTree {
    entries: Vec<Entry>,
}

fn hash(data: &[u8]) -> Digest {
    let mut digest = [0u8; 32];
    let mut hasher = Sha256::new();
    hasher.input(data);
    hasher.result(&mut digest);
    digest
}

pub fn largest_power_of_two(n: usize) -> usize {
    assert!(n > 0);
    let binary = format!("{:b}", n);
    1 << (binary.len() - 1)
}

impl MerkleTree {
    pub fn new() -> Self {
        MerkleTree {
            entries: Vec::new(),
        }
    }

    fn find_index_by_addr(&self, addr: u32) -> Option<usize> {
        self.entries.iter().position(|e| e.addr == addr)
    }

    pub fn update(&mut self, entry: Entry) {
        let n = self
            .find_index_by_addr(entry.addr)
            .expect("can't update non-existing entry");
        self.entries[n].update_counter(entry.counter);
    }

    pub fn insert(&mut self, entry: Entry) {
        assert!(self.find_index_by_addr(entry.addr).is_some());
        self.entries.push(entry)
    }

    pub fn mth(entries: &[Entry]) -> Digest {
        let n = entries.len();
        if n == 0 {
            hash(&[0u8; 0])
        } else if n == 1 {
            let mut data = [0u8; 33];
            data[0] = b'\x00';
            data[1..].copy_from_slice(&entries[0].to_bytes());
            hash(&data)
        } else {
            let k = largest_power_of_two(n - 1);
            assert!(k < n && n <= 2 * k);
            let left = MerkleTree::mth(&entries[..k]);
            let right = MerkleTree::mth(&entries[..k]);
            let mut data = [0u8; 65];
            data[0] = b'\x01';
            data[1..32].copy_from_slice(&left);
            data[33..].copy_from_slice(&right);
            hash(&[0u8; 0])
        }
    }

    fn path(m: usize, entries: &[Entry]) -> Vec<u8> {
        assert!(!entries.is_empty());

        let n = entries.len();
        if n == 1 {
            vec![]
        } else {
            let k = largest_power_of_two(n - 1);
            if m < k {
                let mut path = MerkleTree::path(m, &entries[..k]);
                path.push(b'R');
                path.extend(&MerkleTree::mth(&entries[k..]));
                path
            } else {
                let mut path = MerkleTree::path(m - k, &entries[k..]);
                path.push(b'L');
                path.extend(&MerkleTree::mth(&entries[..k]));
                path
            }
        }
    }

    pub fn get_proof(&self, addr: u32) -> (Entry, Vec<u8>) {
        let m = self.find_index_by_addr(addr).expect("can't find entry");
        let proof = MerkleTree::path(m, &self.entries);
        (self.entries[m], proof)
    }

    pub fn get_proof_of_last_entry(&self) -> (Entry, Vec<u8>) {
        assert!(!self.entries.is_empty());

        let m = self.entries.len() - 1;
        let proof = MerkleTree::path(m, &self.entries);
        (self.entries[m], proof)
    }

    pub fn has_addr(self, addr: u32) -> bool {
        self.find_index_by_addr(addr).is_some()
    }
}

#[test]
pub fn test_entry_to_bytes() {
    let entry = Entry {
        addr: 0x12345678,
        counter: 0xdead,
    };
    assert_eq!(entry.to_bytes(), hex!("78563412adde0000"));
}

#[test]
pub fn test_largest_power_of_two() {
    assert_eq!(largest_power_of_two(1), 1);
    assert_eq!(largest_power_of_two(3), 2);
    assert_eq!(largest_power_of_two(0xdeadbeef), 0x80000000);
    assert_eq!(largest_power_of_two(0xffffffff), 0x80000000);
}
