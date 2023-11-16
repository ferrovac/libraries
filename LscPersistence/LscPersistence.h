#ifndef LscPersistence_H
#define LscPersistence_H

#include <utility>
#include <type_traits>

template <typename T>
struct has_addition_operator {
    template <typename U>
    static auto test(U* p) -> decltype(std::declval<U>() + std::declval<U>(), std::true_type());

    template <typename U>
    static std::false_type test(...);

    static constexpr bool value = decltype(test<T>(nullptr))::value;
};


template<typename T>
class Persistent {
    private:
        T object;
        const char* filename;

    public:
        template <typename... Args>
        Persistent(const char* FileName, Args&&... args) 
            : object(std::forward<Args>(args)...), filename(FileName) {
        }

        void testfunction() {
            // Function implementation
        }
        operator T() const {
            return object;
        }

        template <typename U = T>
        typename std::enable_if<has_addition_operator<U>::value, Persistent>::type
            operator+(const Persistent& rhs) const {
            return Persistent(object + rhs.object);
        }
        
};


#endif
