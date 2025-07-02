# CMakeLists.txt (raiz do projeto)
cmake_minimum_required(VERSION 3.16.0)

# (Opcional) Incluir diretórios de componentes adicionais:
list(APPEND EXTRA_COMPONENT_DIRS "components/esp32-camera" "components/esp-tflite-micro")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
# Definir o nome do projeto
project(SPheReEdge)
