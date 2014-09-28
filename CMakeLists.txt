project(gnuspeech_x)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "-std=c++11 -Wall")

#aux_source_directory(src SRC_LIST)
#add_executable(${PROJECT_NAME} ${SRC_LIST})

include_directories(
    src
    src/letter_to_sound
    src/main_dict
    src/text_parser
    src/trm
    src/trm_control_model
    src/xml
)

add_definitions(-DVERBOSE)

add_library(gnuspeechx SHARED
    src/Dictionary.cpp src/Dictionary.h
    src/Exception.h
    src/Log.h
    src/number_pronunciations.h
    src/Text.h

    src/letter_to_sound/apply_stress.cpp src/letter_to_sound/apply_stress.h
    src/letter_to_sound/check_word_list.cpp src/letter_to_sound/check_word_list.h
    src/letter_to_sound/clusters.h
    src/letter_to_sound/ends_with.cpp src/letter_to_sound/ends_with.h
    src/letter_to_sound/final_s.cpp src/letter_to_sound/final_s.h
    src/letter_to_sound/ie_to_y.cpp src/letter_to_sound/ie_to_y.h
    src/letter_to_sound/insert_mark.cpp src/letter_to_sound/insert_mark.h
    src/letter_to_sound/isp_trans.cpp src/letter_to_sound/isp_trans.h
    src/letter_to_sound/letter_to_sound.cpp src/letter_to_sound/letter_to_sound.h
    src/letter_to_sound/long_medial_vowels.cpp src/letter_to_sound/long_medial_vowels.h
    src/letter_to_sound/mark_final_e.cpp src/letter_to_sound/mark_final_e.h
    src/letter_to_sound/medial_s.cpp src/letter_to_sound/medial_s.h
    src/letter_to_sound/medial_silent_e.cpp src/letter_to_sound/medial_silent_e.h
    src/letter_to_sound/member.cpp src/letter_to_sound/member.h
    src/letter_to_sound/stresstables.h
    src/letter_to_sound/suffix.cpp src/letter_to_sound/suffix.h
    src/letter_to_sound/syllabify.cpp src/letter_to_sound/syllabify.h
    src/letter_to_sound/tail.h
    src/letter_to_sound/vowel_before.cpp src/letter_to_sound/vowel_before.h
    src/letter_to_sound/word_to_patphone.cpp src/letter_to_sound/word_to_patphone.h

    src/main_dict/MainDictionary.cpp src/main_dict/MainDictionary.h
    src/main_dict/suffix_list.h

    src/text_parser/abbreviations.h
    src/text_parser/number_parser.cpp src/text_parser/number_parser.h
    src/text_parser/special_acronyms.h
    src/text_parser/TextParser.cpp src/text_parser/TextParser.h

    src/trm_control_model/Category.cpp src/trm_control_model/Category.h
    src/trm_control_model/DriftGenerator.cpp src/trm_control_model/DriftGenerator.h
    src/trm_control_model/ConfigFile.cpp src/trm_control_model/ConfigFile.h
    src/trm_control_model/Controller.cpp src/trm_control_model/Controller.h
    src/trm_control_model/Equation.cpp src/trm_control_model/Equation.h
    src/trm_control_model/EventList.cpp src/trm_control_model/EventList.h
    src/trm_control_model/FormulaSymbol.cpp src/trm_control_model/FormulaSymbol.h
    src/trm_control_model/IntonationPoint.cpp src/trm_control_model/IntonationPoint.h
    src/trm_control_model/Model.cpp src/trm_control_model/Model.h
    src/trm_control_model/Parameter.cpp src/trm_control_model/Parameter.h
    src/trm_control_model/Phone.cpp src/trm_control_model/Phone.h
    src/trm_control_model/Rule.cpp src/trm_control_model/Rule.h
    src/trm_control_model/StringParser.cpp src/trm_control_model/StringParser.h
    src/trm_control_model/Transition.cpp src/trm_control_model/Transition.h

    src/xml/SimpleXMLParser.cpp src/xml/SimpleXMLParser.h
)

add_library(gnuspeechtrm SHARED
    src/trm/BandpassFilter.cpp src/trm/BandpassFilter.h
    src/trm/FIRFilter.cpp src/trm/FIRFilter.h
    src/trm/NoiseFilter.cpp src/trm/NoiseFilter.h
    src/trm/NoiseSource.cpp src/trm/NoiseSource.h
    src/trm/RadiationFilter.cpp src/trm/RadiationFilter.h
    src/trm/ReflectionFilter.cpp src/trm/ReflectionFilter.h
    src/trm/SampleRateConverter.cpp src/trm/SampleRateConverter.h
    src/trm/Throat.cpp src/trm/Throat.h
    src/trm/Tube.cpp src/trm/Tube.h
    src/trm/WavetableGlottalSource.cpp src/trm/WavetableGlottalSource.h
)

add_executable(gnuspeech_x
    src/main.cpp
)
target_link_libraries(gnuspeech_x -L. -lgnuspeechx -lgnuspeechtrm -lsndfile)

add_executable(gnuspeech_trm
    src/trm/gnuspeech_trm.cpp
)
target_link_libraries(gnuspeech_trm -L. -lgnuspeechtrm -lsndfile)

add_dependencies(gnuspeech_x gnuspeechx gnuspeechtrm)
add_dependencies(gnuspeech_trm gnuspeechtrm)
