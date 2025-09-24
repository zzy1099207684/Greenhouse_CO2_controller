//
// Created by zhiyo on 2025/4/24.
//

//using example
//Json_handler json_handler;
//json_handler.get_value_from_key(temp_buffer, "key");
//if (json_handler.get_result() != JSONSuccess) return;
//char *value = json_handler.get_final_result();

#ifndef JSON_HANDLER_H
#define JSON_HANDLER_H
#include "core_json.h"
#define JSON_BUFFER_SIZE 256

class Json_handler {
    private:
        char *value = nullptr; 
        size_t valueLength;
        JSONStatus_t result;
        char final_result[JSON_BUFFER_SIZE] = {0};// truth value for key
    public:
        Json_handler() = default;


        // Constructor
        void get_value_from_key(char *buffer, const char *queryKey);

        char *get_final_result();

        [[nodiscard]] JSONStatus_t get_result() const;
};



#endif //JSON_HANDLER_H
