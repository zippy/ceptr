use std::rand;
use std::rand::RngUtil;

fn main() {
    request("May I speak with you about things unspoken?  Help me touch that dance undanced.  Please, can we play the play we have yet to play, and I promise the softness of my hands, and real things really made.");
}

fn transform(txt: &str) -> ~[&str]{
    let mut v: ~[&str] = txt.word_iter().collect();
/*    let l=v.len();
    let mut i=l;
    let mut rng = rand::rng();
    loop {
        if i==0 {break;}
        v.swap(i-1,rng.gen_uint_range(0,l));
        i = i -1;
    }*/
    let mut rng = rand::rng();
    rng.shuffle_mut(v);
    v
}

fn request(txt: &str) {
    let words:~[&str] = transform(txt);
    express(words);
}

fn express(words: ~[&str]) {
    for w in words.iter() {
        print(*w);
        print(" ");
    }
    print("\n");
}

pub trait MutableVector<'self, T> {
     fn shuffle(self);
}
/*
impl<'self, T> MutableVector<'self, T> for &'self mut [T] {
    fn shuffle(self) {
        let l=self.len();
        let mut i=l;
        let mut rng = rand::rng();
        loop {
            if i==0 {break;}
            self.swap(i-1,rng.gen_uint_range(0,l));
        i = i -1;
        }
    }
}*/
