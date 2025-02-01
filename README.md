# AnalyzeLog

Небольшая утилита для анализа логов обращений к серверу в формате [access.log](https://ru.wikipedia.org/wiki/Access.log).

Данная утилита способна:
* Находить все запросы, закончившиеся с ошибкой (с кодом `5XX`)
* Определять самые частые запросы, завершившиеся с ошибкой
* Определять промежуток времени заданной длины с максимальным числом запросов

## Сборка
Для сборки нужен CMake 3.5.0+ и компилятор с поддержкой C++23. Собрать можно командой:

```bash
cmake -B ./build & cmake --build ./build
```

## Использование
Утилита может парсить строки в формате access.log, то есть:
`<remote_addr> - - [<local_time>] "<request>" <status> <bytes_send>`

Например:
`tspc08.dat.bnl.gov - - [13/Jul/1995:13:32:47 -0400] "GET /shuttle/countdown/count70.gif HTTP/1.0" 200 46573`

Строки, не подходящие под формат, будут проигнорированы (специальной командой можно все такие строки вывести в файл).

Шаблон использования:
`AnalyzeLog [OPTIONS] logs_filename`

### Список команд
| Короткий аргумент | Длинный аргумент              | Значение по умолчанию   | Описание |
|-------------------|-------------------------------|-------------------------|----------|
| `-o path`         | `--output=path`               |                         | Путь к файлу, в который будут записаны запросы с ошибками. Если файл не указан, анализ запросов с ошибками не выполняется. |
| `-p`              | `--print`                     |                         | Продублировать вывод запросов с ошибками в `stdout` (стандартный поток вывода / терминал) |
| `-s n`            | `--stats=n`                   | `10`                    | Вывести `n` самых частых запросов, завершившихся со статус кодом `5XX`, в порядке их частоты. Если значение `0`, то не запросы не выводятся. |
| `-w t`            | `--window=t`                  | `0`                     | Найти и вывести промежуток (окно) времени длительностью t секунд, в которое количество запросов было максимально. Eсли t равно 0, расчет не производится. |
| `-f t`            | `--from=time`                 | Наименьшее время в логе | Время в формате [timestamp](https://www.unixtimestamp.com), начиная с которого происходит анализ данных. |
| `-t t`            | `--to=time`                   | Наибольшее время в логе | Время в формате [timestamp](https://www.unixtimestamp.com), до которого происходит анализ данных (включительно) |
| `-i path`         | `--invalid-lines-output=path` |                         | Путь к файлу, в который будут записаны все строки с ошибками (которые не получилось распарсить) |
| `-h`              | `--help`                      |                         | Игнорировать остальные команды и показать справку

## Примечания
Эта утилита — первая лабораторная работа на курсе "Основы программирования" в ИТМО (ИС), а также мой первый относительно большой проект на C++.

Самописное подобие вектора, строки и сортировки — следствие неофициального ограничения на стандартные контейнеры и алгоритмы. Самописный парсинг формата логов и timestamp — просто желание развлечься и попрактиковаться.

* [Оригинальное задание](task.md)
* Для тестов использовались открытые [логи сервера NASA](https://drive.google.com/file/d/1jjzMocc0Rn9TqkK_51Oo93Fy78KYnm2i/view).
