//
//  main.cpp
//  TSN_2024_observeur
//
//  Created by pardo jérémie on 08/02/2024.
//

#include <iostream>
#include "observeur.hpp"

struct Toto {
    void fn0(int i) { std::cout << "fn0\n"; }
    void fn1(int i) { std::cout << "fn1\n"; }
    void fn2(int i) { std::cout << "fn2\n"; }
    void fn3(int i) { std::cout << "fn3\n"; }
    void fn4(int i) { std::cout << "fn4\n"; }
    void fn5(int i) { std::cout << "fn5\n"; }
    void fn6(int i) { std::cout << "fn6\n"; }
};

int main(int argc, const char * argv[]) {
    obs::Subject<void(int)> subject;

    Toto t0;

    // ok (lambda)
    subject.subscribe([](int i){
        std::cout << "lambda\n";
    });

    // ok (pointeur + fonction)
    subject.subscribe(&t0, &Toto::fn0);

    // ok mais pas de notification car l'objet 't' sera detruit
    {
        auto t = std::make_shared<Toto>();
        subject.subscribe(t, &Toto::fn1);
    }

    // ok (shared_ptr + function)
    auto t1 = std::make_shared<Toto>();
    subject.subscribe(t1, &Toto::fn2);

    // ok mais pas de notification car l'objet 't2' sera detruit
    {
        auto t = std::make_shared<Toto>();
        subject.subscribe(std::weak_ptr(t), &Toto::fn3);
    }

    // inscription + desinscription
    auto t2 = std::make_shared<Toto>();
    auto id = subject.subscribe(t2, &Toto::fn4);

    subject.unsubscribe(id);

    // inscription sur le meme id
    id = 100;
    subject.subscribe(t2, &Toto::fn5, id);
    subject.subscribe(t2, &Toto::fn6, id);

    subject.unsubscribe(id);

    // notification
    subject(8);

    /**
     * attendu:
     *
     * (pas de fn6 car observeur desinscrit)
     * (pas de fn5 car observeur desinscrit)
     * (pas de fn4 car observeur desinscrit)
     * (pas de fn3 car 't' detruit avant l'appel : 'subject(8);')
     * fn2
     * (pas de fn1 car 't' detruit avant l'appel : 'subject(8);')
     * fn0
     * lambda
     */

    return 0;
}
