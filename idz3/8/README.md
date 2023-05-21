# Работа на 8

!tldr; Тут важен порядок запуска: сначала все клиенты, а уже затем все наблюдатели

TCP соединение, клиент и сервер самостоятельно выводят информацию о себе.

Красивая студентка знает сколько у неё поклонников и ждёт валентинки от каждого из них. Когда все валентинки приходят, она выбирает самую впечатляющую и уведомляет автора о своём согласии пойти с ним на свидание. Другим поклонникам она посылает сообщение об отказе.
Еще у студентки есть строгий отец, который следит за каждым его шагом и очень не любит её ухажеров, поэтому она вынуждена отчитываться ему о каждом своём действии. У отца много работы, поэтому он не может постоянно следить за своей дочерью, так что он нанял детективов для решения этой проблемы. Детективы могут приходить и уходить в течении для. У отца ограничен бюджет, так что детективов может быть максимум 100.

### Запуск
Программа компилируется через Makefile
```bash
make
```

Преполагается, что вначале будет запущен сервер `beauty`, с аргументами в виде порта и количества поклонников `admirer` (клиентов), затем запускаются все клиенты, а уже затем наблюдатели. При ином порядке запуска поведение программы не определено.
Во время работы сервера он два раза засыпает на 10 секунд, чтобы можно было отключить старых наблюдателей (через Ctrl + C) и подключить новых. Это сделано для удобства проверки.

**Пример**:
```c
./beauty 5000 3
./admirer 127.0.1.1 5000
./father 127.0.1.1 5000
```

### Описание

