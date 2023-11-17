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
        PersistentTracker(){}
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
    protected:
        const char* filename;
        unsigned long maxNumberOfBackLogEntries;
        unsigned long numberOfBackLogEntries;
        unsigned long minIntervall;
        unsigned long lastWrite;
        unsigned long fileSize;
        bool onSecondBank;
        bool hasError;
        bool onlyLogChanges;
    private:
        
    public:
        virtual void writeObjectToSD() = 0;
        virtual void readObjectFromSD() = 0;
        virtual void init() = 0;
        static volatile bool initComplete;
    
        BasePersistent(const char* filename)
            :   filename(filename),
                maxNumberOfBackLogEntries(10000000),
                numberOfBackLogEntries(0),
                minIntervall(1000),
                lastWrite(millis()),
                fileSize(0),
                onSecondBank(false),
                hasError(false),
                onlyLogChanges(false)
            {
            PersistentTracker::getInstance().registeInstance(this);
        }
        unsigned long getSize(){
            return fileSize;
        }
        bool getErrorState(){
            Serial.println("Bank: " + String(onSecondBank));
            Serial.println("ERROR: " + String(hasError));
            Serial.println("size: " + String(fileSize));
            Serial.println("elements: " + String(numberOfBackLogEntries));
            Serial.println("init: " + String(initComplete));
            return hasError;
        }
        
        
};

template<typename T>
class Persistent : public BasePersistent  {
    static_assert(std::is_pod<T>::value, "Only types with no dynamic memory are allowed!");
    private:
        T object;
        T lastObjectValue;
    public:
        unsigned long getNumbersOfEntries(){
            return numberOfBackLogEntries;
        }
        
        void setMaxNumberOfBackLogEntries(unsigned long number){
            if(number < 20) number = 20;
            maxNumberOfBackLogEntries = number;
        }
        void setMinIntervall(unsigned long intervall){
            minIntervall = intervall;
        }

        
        void writeObjectToSD() override {
            if(!initComplete) return;
            if(millis()-lastWrite < minIntervall) return;
            String currentBankName = String(filename);
            if(numberOfBackLogEntries == maxNumberOfBackLogEntries) onSecondBank = true;
            if(numberOfBackLogEntries >= 2 * maxNumberOfBackLogEntries){
                if(onSecondBank){
                    if(SD.exists(filename)) SD.remove(filename);
                    onSecondBank = false;
                }else{
                    if(SD.exists(String(filename) + "_2")) SD.remove(String(filename) + "_2");
                    onSecondBank = true;
                }
                numberOfBackLogEntries = maxNumberOfBackLogEntries;
            }

            if(onSecondBank) currentBankName += "_2";
            auto file = SD.open(currentBankName,FILE_WRITE);
            if(!file) return;
            file.write(reinterpret_cast<const char*>(&object),sizeof(object));
            fileSize = file.size();
            numberOfBackLogEntries++;
            file.close();
            lastWrite = millis();
        }
        void readObjectFromSD() override {
            String currentBankName = String(filename);
            if(onSecondBank) currentBankName += "_2";

            if(!initComplete) return;

            if(!SD.exists(currentBankName)) return;
            auto file = SD.open(currentBankName, FILE_READ);
            if (!file) return;
            if(file.size() >= sizeof(object)){
                file.seek(file.size()- sizeof(object));
            }else{
                return;
            }
            file.read(reinterpret_cast<uint8_t*>(&object), sizeof(object));
            file.close();
        }
        T getElement(size_t index){
            if(!initComplete) return object;
            if(index >= numberOfBackLogEntries) return object;
            
            if(numberOfBackLogEntries <= maxNumberOfBackLogEntries && !onSecondBank){
                if(!SD.exists(filename)) return object;
                auto file = SD.open(filename, FILE_READ);
                if(!file) return object;
                T ret;
                file.seek(index * sizeof(object));
                file.read(reinterpret_cast<uint8_t*>(&ret), sizeof(object));
                file.close();
                return ret;
            }

            if(index >= maxNumberOfBackLogEntries){
                if(!onSecondBank){
                    if(!SD.exists(filename)) return object;
                    auto file = SD.open(filename, FILE_READ);
                    if(!file) return object;
                    T ret;
                    file.seek((index - maxNumberOfBackLogEntries) * sizeof(object));
                    file.read(reinterpret_cast<uint8_t*>(&ret), sizeof(object));
                    file.close();
                    return ret;
                }else{
                    if(!SD.exists(String(filename) + "_2")) return object;
                    auto file = SD.open(String(filename) + "_2", FILE_READ);
                    if(!file) return object;
                    T ret;
                    file.seek((index - maxNumberOfBackLogEntries) * sizeof(object));
                    file.read(reinterpret_cast<uint8_t*>(&ret), sizeof(object));
                    file.close();
                    return ret;
                }
            }else{
                if(!onSecondBank){
                    if(!SD.exists(String(filename) + "_2")) return object;
                    auto file = SD.open(String(filename) + "_2", FILE_READ);
                    if(!file) return object;
                    T ret;
                    file.seek(index * sizeof(object));
                    file.read(reinterpret_cast<uint8_t*>(&ret), sizeof(object));
                    file.close();
                    return ret;
                }else{
                    if(!SD.exists(filename)) return object;
                    auto file = SD.open(filename , FILE_READ);
                    if(!file) return object;
                    T ret;
                    file.seek(index * sizeof(object));
                    file.read(reinterpret_cast<uint8_t*>(&ret), sizeof(object));
                    file.close();
                    return ret;
                }
            }
        }
        
        T operator[](size_t index) {
            return getElement(index);
        }

        void init() override{
            bool firstBankFileOk = false;
            if (!SD.begin(31)){
                hasError = true;
                return;
            }

            if(SD.exists(filename)){ //we start checking the base bank file
                auto file = SD.open(filename, FILE_READ);
                if(!file){
                    hasError = true;
                    return;
                }else{
                    if(file.size() % sizeof(object) != 0){ //the size of the existing first bank file is not correct
                        file.close();
                        SD.remove(filename);
                    }else{ // the first bank file is ok
                        fileSize = file.size();
                        numberOfBackLogEntries = fileSize / sizeof(object);
                        firstBankFileOk = true;
                        file.close();
                    }
                }
            }
            //Check if we are on the second Bank
            if(SD.exists(String(filename) + "_2")){
                auto file = SD.open(String(filename) + "_2", FILE_READ);
                if(!file){
                    hasError = true;
                    return;
                }else{
                    unsigned long  secondFileSize = file.size();
                    file.close();
                    if(secondFileSize % sizeof(object) != 0 && !firstBankFileOk){ //Make sure the second bank file has to correct length 
                        SD.remove(String(filename) + "_2"); //if the lenght is not correct we delete the file.
                    }else{ //the second bank file is ok -> we can set the number of elements in the file
                        numberOfBackLogEntries += secondFileSize / sizeof(object);
                        if(secondFileSize < fileSize){
                            fileSize = secondFileSize;
                            onSecondBank = true;
                        }
                    }
                }
                
            } //we have checked all conditions for the second bank file
            if(fileSize == 0) writeObjectToSD();
            readObjectFromSD();
        }
        
        template <typename... Args>
        Persistent(const char* FileName, Args&&... args) 
            :   BasePersistent(FileName),   
                object(std::forward<Args>(args)...),
                lastObjectValue(object)
                
                
            {
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
