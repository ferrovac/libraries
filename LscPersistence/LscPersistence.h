#ifndef LscPersistence_H
#define LscPersistence_H

#include <utility>
#include <type_traits>
#include <vector>
#include <SPI.h>
#include <SD.h>


template <typename T>
struct has_assignment_operator {
    template <typename U>
    static auto test(U* p) -> decltype(std::declval<U&>() = std::declval<U>(), std::true_type());

    template <typename U>
    static std::false_type test(...);

    static constexpr bool value = decltype(test<T>(nullptr))::value;
};

template <typename T>
struct has_addition_operator {
    template <typename U>
    static auto test(U* p) -> decltype(std::declval<U>() + std::declval<U>(), std::true_type());

    template <typename U>
    static std::false_type test(...);

    static constexpr bool value = decltype(test<T>(nullptr))::value;
};

template <typename T>
struct has_multiplication_operator {
    template <typename U>
    static auto test(U* p) -> decltype(std::declval<U>() * std::declval<U>(), std::true_type());

    template <typename U>
    static std::false_type test(...);

    static constexpr bool value = decltype(test<T>(nullptr))::value;
};

template <typename T>
struct has_subtraction_operator {
    template <typename U>
    static auto test(U* p) -> decltype(std::declval<U>() - std::declval<U>(), std::true_type());

    template <typename U>
    static std::false_type test(...);

    static constexpr bool value = decltype(test<T>(nullptr))::value;
};

template <typename T>
struct has_division_operator {
    template <typename U>
    static auto test(U* p) -> decltype(std::declval<U>() / std::declval<U>(), std::true_type());

    template <typename U>
    static std::false_type test(...);

    static constexpr bool value = decltype(test<T>(nullptr))::value;
};

template <typename T>
struct has_modulo_operator {
    template <typename U>
    static auto test(U* p) -> decltype(std::declval<U>() % std::declval<U>(), std::true_type());

    template <typename U>
    static std::false_type test(...);

    static constexpr bool value = decltype(test<T>(nullptr))::value;
};

class BasePersistent;
class PersistentTracker{
    private:
        std::vector<BasePersistent*> tracker;
        PersistentTracker(){

        }
    public:
        static PersistentTracker& getInstance(){
            static PersistentTracker instance;
            return instance;
        }
        void registeInstance(BasePersistent* Instance){
            tracker.push_back(Instance);
        }
        std::vector<BasePersistent*>* getInstances(){
            return &tracker;
        }
};

class BasePersistent{
    private:
        
    public:
        virtual void writeObjectToSD() = 0;
        virtual void readObjectFromSD() = 0;
        virtual void openFile() = 0;
        static volatile bool initComplete;
        

        BasePersistent(){
            PersistentTracker::getInstance().registeInstance(this);
        }
        
};

template<typename T>
class Persistent : BasePersistent {
    private:
        T object;
        const char* filename;
        //SDLib::File file;

    public:
        size_t getSize(){
            return sizeof(object);
        }
        void writeObjectToSD() override {
            if(!initComplete) return;
            // Function implementation
           // if (!SD.begin(31)){
            //}else{
                auto file = SD.open(filename,FILE_WRITE);
                //file.seek(file.size());
                size_t size = sizeof(object);
                const char* bytePointer = reinterpret_cast<const char*>(&object);
                
                for (size_t i = 0; i < size; ++i) {
                    // Access each byte using pointer arithmetic
                    char currentByte = *(bytePointer + i);

                    // Now you can do something with the current byte
                  //  Serial.print(currentByte, HEX); // Print byte in hexadecimal format
                   // Serial.print(" ");
                } 
                //Serial.println("Begin writte: " + String(file.size()));
                file.write(reinterpret_cast<const char*>(&object),sizeof(object));
                //file.flush();
                //Serial.println("written: " + String(file.size()));
                
                file.close();
            //} 
        }
        void readObjectFromSD() override {
            if(!initComplete) return;
            // Function implementation
            
            //if (!SD.begin(31)){
                //Serial.println("Failed to init sd");
            //}else{
                if(SD.exists(filename)){
                    auto file = SD.open(filename, FILE_READ);
                    if (file) {
                     
                        if (true) {
                            
                            char bytes[sizeof(object)];
                            if(file.size() > sizeof(object)) file.seek(file.size()- sizeof(object));
                            file.read(reinterpret_cast<uint8_t*>(&object), sizeof(object));
                            file.close();
                            size_t size = sizeof(object);

                for (size_t i = 0; i < size; ++i) {
                    // Access each byte using pointer arithmetic
                    char currentByte = bytes[i];

                    // Now you can do something with the current byte
                   // Serial.print(currentByte, HEX); // Print byte in hexadecimal format
                   // Serial.print(" ");
                }
                      //      Serial.println("Read");
                        } else {
                        //    Serial.println("File size does not match object size");
                        }
                        //file.close();
                    } else {
                       // Serial.println("Failed to open file for reading");
                    }
                }else{
                    //Serial.println("Failed to find file for reading");
                    
                }
            //} 
        }
        void openFile() override{
            readObjectFromSD();
            Serial.println("initial read: " + String(object,15));
        }
        
        template <typename... Args>
        Persistent(const char* FileName, Args&&... args) 
            : object(std::forward<Args>(args)...), filename(FileName) {
                if(initComplete){
                    if(!SD.exists(filename)){
                         writeObjectToSD();
                    }else{
                        readObjectFromSD();
                    }
                }    
        }
        ~Persistent(){

        }

        operator T() const {
            return object;
        }

        template <typename U = T>
        typename std::enable_if<has_assignment_operator<U>::value>::type
            operator= (const U& rhs)  {
            object = static_cast<T>(rhs);
            if(initComplete) writeObjectToSD();
        }

        template <typename U = T>
        typename std::enable_if<has_addition_operator<U>::value>::type
            operator+= (const U& rhs)  {
            *this = *this + rhs;
        }

        template <typename U = T>
        typename std::enable_if<has_subtraction_operator<U>::value>::type
            operator-= (const U& rhs)  {
            *this = *this - rhs;
        }

        template <typename U = T>
        typename std::enable_if<has_multiplication_operator<U>::value>::type
            operator*= (const U& rhs)  {
            *this = *this * rhs;
        }

        template <typename U = T>
        typename std::enable_if<has_division_operator<U>::value>::type
            operator/= (const U& rhs)  {
            *this = *this / rhs;
        }

        template <typename U = T>
        typename std::enable_if<has_modulo_operator<U>::value>::type
            operator%= (const U& rhs)  {
            *this = *this % rhs;
        }

        
};


#endif
