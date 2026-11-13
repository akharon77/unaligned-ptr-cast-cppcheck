# cppcheck

## Описание проекта

Низкоуровневый код часто работает с байтовыми буферами:

* при чтении сетевых пакетов,
* при разборе бинарных форматов,
* и т.д.

В таких задачах возникает желание интерпретировать часть байтового буфера как объект конкретного типа:

```cpp
   Header *header = reinterpret_cast<Header *>(buffer);
```

Но наличия достаточного количества байтов недостаточно. Адрес также должен удовлетворять требованиям выравнивания целевого типа. Некорректный доступ к не выравненному объекту может проявляться по-разному: от снижения производительности до аппаратной ошибки. Кроме того, такой доступ нарушает предположения, на которые вправе опираться компилятор, и приводит к UB. Проблема может зависеть от архитектуры, параметров сборки и оптимизаций. Статическая проверка помогает обнаружить опасное место до запуска программы и привлекает внимание к неявному предположению разработчика о выравнивании.

Предлагается реализация новой проверки pointerCastAlignment в Cppcheck для поиска потенциально небезопасных приведений указателей к типам с более строгим выравниванием.

### Зачем cppcheck

По сравнению с clang-tidy, cppcheck быстрее собирается и быстрее выполняет проверки, выступая некоторой лёгкой альтернативой для быстрой интеграции в CI.

### Суть доработки

В базовом Cppcheck отсутствовал контроль выравнивания при приведении указателей, хотя разыменование недостаточно выровненного указателя (например, `char* -> uint32_t*`) приводит к UB.
Проверка pointerCastAlignment построена по следующей консервативной схеме:

* Сравнение выравнивания типов - отсекает заведомо безопасные приведения, где требования к выравниванию не повышаются.
* Небольшой анализ происхождения указателя - смотрит, откуда пришел указатель. На данный момент подерживается только обработка случая с выделенным на стеке буфером.

Диагностика не выдаётся только тогда, когда безопасность доказана: базовый объект имеет достаточный и кратный целевому alignas; все смещения известны на этапе компиляции, а итоговый байтовый сдвиг кратен целевому выравниванию. Во всех остальных случаях (неизвестное происхождение, динамическое смещение или недостаточное выравнивание) выдаётся предупреждение pointerCastAlignment.

## Презентация

[Ссылка](https://docs.google.com/presentation/d/1vfzCb6PcbjtUhGF7Vk3Ig3CqOhUGiwXpLJ8cmvQXmNo/edit?usp=sharing)

## Сборка

```bash
   cmake -S . -B build
   cmake --build build
```

## Запуск на примере

```bash
   ./build/bin/cppcheck --enable=warning check_aligned.cpp
```

Ожидаемый вывод:

```bash
  Checking check_aligned.cpp ...
  check_aligned.cpp:3:16: warning: Casting from char * (alignment 1) to unsigned int * (alignment 4) can result in a misaligned pointer; dereferencing it is undefined behavior. [pointerCastAlignment]
    auto *ptr1 = reinterpret_cast<uint32_t *>(buf1);
                 ^
  check_aligned.cpp:13:16: warning: Casting from char * (alignment 1) to unsigned int * (alignment 4) can result in a misaligned pointer; dereferencing it is undefined behavior. [pointerCastAlignment]
    auto *ptr4 = static_cast<uint32_t *>(static_cast<void *>(&buf2[1]));
                 ^
  check_aligned.cpp:16:16: warning: Casting from char * (alignment 1) to const unsigned int * (alignment 4) can result in a misaligned pointer; dereferencing it is undefined behavior. [pointerCastAlignment]
    auto *ptr5 = reinterpret_cast<const uint32_t *>(buf1);
                 ^
  check_aligned.cpp:20:16: warning: Casting from unsigned int * (alignment 4) to unsigned long * (alignment 8) can result in a misaligned pointer; dereferencing it is undefined behavior. [pointerCastAlignment]
    auto *ptr6 = reinterpret_cast<uint64_t*>(buf3);
                 ^
```
