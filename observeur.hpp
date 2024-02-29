//
//  observeur.hpp
//  TSN_2024_observeur
//
//  Created by pardo jérémie on 08/02/2024.
//

#ifndef observeur_hpp
#define observeur_hpp

#include <stdio.h>
#include <unordered_map>

namespace obs {

    template<typename Signature>
    struct Subject;

    template<typename... Args>
    class Subject<void(Args...)> final
    {
    public:
        using id_type = std::uint32_t;

    private:
        /**
         * Permet de generer un identifiant unique pour chaque observeur.
         */
        static id_type getId() {
            static id_type id {};
            return id++;
        }

    public:
        template<typename T>
        using member_signature_type = void (T::*)(Args...);
        using free_signature_type = void (Args...);

        using observer_type = std::function<free_signature_type>;

        Subject() = default;
        ~Subject() = default;

        // non copiable
        Subject(const Subject&) = delete;
        Subject& operator=(const Subject&) = delete;

        // moveable
        Subject(Subject &&) noexcept = default;
        Subject& operator=(Subject &&) noexcept = default;

        /**
         * Permet d'inscrire un observeur a partir d'une lambda
         *
         * subject.subscribe([](){
         *     std::cout << "lambda\n";
         * });
         */
        id_type subscribe(const observer_type& observer, const id_type& id = getId())
        {
            m_observers.insert({id,observer});
            return id;
        }

        /**
         * Permet d'inscrire un observeur a partir d'une classe et une methode.
         *
         * NOTE:
         * L'appelant doit garantir que "object" n'est pas detruit pendant la duree de vie de "Subject".
         *
         * Exemple:
         * Toto t;
         * subject.subscribe(&t, &Toto::function_name);
         */
        template<typename Class>
        id_type subscribe(Class* object, member_signature_type<Class> method, const id_type& id = getId())
        {
            // utiliser std::invoke() pour appeler la methode 'method'
            return subscribe([=](int i){std::invoke(method,object,i);}, id);
        }

        /**
         * Permet d'inscrire un observeur a partir d'une classe et une methode.
         *
         * NOTE:
         * Le std::shared_ptr<Class> ne doit pas etre stocke dans "m_observers".
         *
         * Exemple:
         * auto t = std::make_shared<Toto>();
         * subject.subscribe(t, &Toto::function_name);
         */
        template<typename Class>
        id_type subscribe(const std::shared_ptr<Class>& object, member_signature_type<Class> method, const id_type& id = getId())
        {
            // utiliser  un std::weak_ptr() pour ne pas incrementer le compteur de reference du std::shared_ptr<Class>

            //subscribe(/* completer */, method, id);
            
            return subscribe(std::weak_ptr(object), method, id);
        }

        /**
         * Permet d'inscrire un observeur a partir d'une classe et une methode.
         *
         * Exemple:
         * auto t = std::make_shared<Toto>();
         * subject.subscribe(std::weak_ptr(t), &Toto::function_name);
         */
        template<typename Class>
        id_type subscribe(const std::weak_ptr<Class>& object, member_signature_type<Class> method, const id_type& id = getId())
        {
            // utiliser std::invoke() pour appeler la methode 'method'
            
            return subscribe([=](int i){if(auto shared_object = object.lock()){std::invoke(method,shared_object,i);}}, id);
            // auto shared_object = object.lock() return null when the weak_ptr cannot be converted to a shared ptr (ie, the weak_ptr referenced object is already deleted).
        }

        /**
         * Permet de desinscrire tous les observeurs pour une cle donnee.
         */
        void unsubscribe(const id_type& key)
        {
            m_observers.erase(key);
        }

        /**
         * Supprimer tous les observeurs.
         */
        void clear()
        {
            m_observers.clear();
        }

        /**
         * Appel tous les observeurs enregistres avec les arguments fournis "args".
         */
        void operator()(const Args&... args) const
        {
            std::for_each(m_observers.cbegin(), m_observers.cend(),[&](const auto& pair){
                pair.second(args...);
            });
        }

    private:
        std::unordered_multimap<id_type, observer_type> m_observers{};
    };
}

#endif /* observeur_hpp */
