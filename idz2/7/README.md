# Работа на 7

Используются POSIX семафоры и POSIX разделяемая память

### Запуск
Программа компилируется через Makefile
```bash
make
```
При запуске `beauty` необходимо указать количество поклонников (дочерних процессов)

Предполагается, что будет запущено столько `admirer`ов, сколько было указано при запуске `beauty`

Весь вывод идёт в консоль

**Пример**:
```c
./beauty 5
./admirer x5
```

### Описание

![Схема](./schema/schema7.png)