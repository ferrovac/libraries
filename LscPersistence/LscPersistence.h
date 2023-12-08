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
struct is_convertable_to_String {
    template <typename U>
    static auto test(U* p) -> decltype(String(std::declval<U>()), std::true_type());

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
        static volatile bool powerFailureImminent;
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
        String filename;
        unsigned long maxNumberOfBackLogEntries;
        unsigned long numberOfBackLogEntries;
        unsigned long minIntervall;
        unsigned long lastWrite;
        unsigned long fileSize;
        bool onSecondBank;
        bool onlyLogChanges;
        bool initialValueLoaded;
    private:
        
    public:
        virtual bool writeObjectToSD() = 0;
        virtual bool readObjectFromSD() = 0;
        virtual bool init() = 0;
        static volatile bool initComplete;
        
    
        BasePersistent(String Filename)
            :   filename(String(Filename)),
                maxNumberOfBackLogEntries(10000000),
                numberOfBackLogEntries(0),
                minIntervall(1000),
                lastWrite(millis()),
                fileSize(0),
                onSecondBank(false),
                onlyLogChanges(false),
                initialValueLoaded(false)
            {
            PersistentTracker::getInstance().registeInstance(this);
            filename.replace(" ","_");
            }
        unsigned long getSize(){
            return fileSize;
        }
        String getStateString(){
            String stateString = "";
            stateString += "filename: " + String(filename) + "\n";
            stateString += "maxNumberOfBackLogEntries: " + String(maxNumberOfBackLogEntries)+ "\n";
            stateString += "numberOfBackLogEntries: " + String(numberOfBackLogEntries)+ "\n";
            stateString += "minIntervall: " + String(minIntervall)+ "\n";
            stateString += "lastWrite: " + String(lastWrite)+ "\n";
            stateString += "fileSize: " + String(fileSize)+ "\n";
            stateString += "onSecondBank: " + String(onSecondBank)+ "\n";
            stateString += "onlyLogChanges: " + String(onlyLogChanges)+ "\n";
            stateString += "initialValueLoaded: " + String(initialValueLoaded)+ "\n";
            stateString += "initComplete: " + String(initComplete)+ "\n";
            return stateString;
        }
};



template<typename T>
class Persistent : public BasePersistent  {
    static_assert(std::is_pod<T>::value, "Only types with no dynamic memory are allowed!");
    private:
        typename std::remove_volatile<T>::type object;
        typename std::remove_volatile<T>::type lastObjectValue;
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

        
        [[nodiscard]] bool writeObjectToSD() override {
            if(!initComplete) return true;
            
            if((millis()-lastWrite < minIntervall) && initialValueLoaded) return true;
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
            if(!file) return true;
            if(PersistentTracker::getInstance().powerFailureImminent) return false;
            file.write(reinterpret_cast<const char*>(&object),sizeof(object));
            fileSize = file.size();
            numberOfBackLogEntries++;
            file.flush();
            file.close();
            lastWrite = millis();
            return false;
        }

        [[nodiscard]] bool readObjectFromSD() override {
            String currentBankName = String(filename);
            Serial.println("Start read: " + currentBankName);
            if(onSecondBank) currentBankName += "_2";

            if(!initComplete) return true;

            if(!SD.exists(currentBankName)) return false;
            auto file = SD.open(currentBankName, FILE_READ);
            if (!file) return true;
            if(file.size() >= sizeof(object)){
                file.seek(file.size() - sizeof(object));
            }else{
                return true;
            }
            file.read(reinterpret_cast<uint8_t*>(&object), sizeof(object));
            file.close();
            return false;
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

        [[nodiscard]] bool init() override{
            bool firstBankFileOk = false;
            if (!SD.begin(31)){
                return true;
            }

            if(SD.exists(filename)){ //we start checking the base bank file
                auto file = SD.open(filename, FILE_READ);
                if(!file){
                    return true;
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
                    return true;
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
            if(!SD.exists(filename)){
                writeObjectToSD();
                Serial.println("created: " + filename);
            } 
            readObjectFromSD();
            initialValueLoaded = true;
            return false;
        }
        
        template <typename... Args>
        Persistent(String FileName, Args&&... args) 
            :   BasePersistent(String(FileName)),   
                object(std::forward<Args>(args)...)
                
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

        T getType(){
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
