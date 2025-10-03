//
// Created by zhiyo on 2025/4/24.
//

#include <cstring>

#include "json_handler.h"



void Json_handler::get_value_from_key(char *buffer, const char *queryKey) {
    const size_t bufferLength = strlen(buffer);
    const size_t queryKeyLength = strlen(queryKey);

    result = JSON_Validate(buffer, bufferLength);
    if (result == JSONSuccess) {
        result = JSON_Search(buffer, bufferLength,
                             queryKey, queryKeyLength,
                             &value, &valueLength);

    }else {
        result = JSONNotFound;
        return;
    }
    strncpy(final_result, value, valueLength);
    final_result[valueLength] = '\0';
}

char * Json_handler::get_final_result() {
    return final_result;
}


JSONStatus_t Json_handler::get_result() const {
    return result;
}
