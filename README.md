# M5StackWatering

## setup

M5StackWatering.inoのSSIDとパスワードを設定してください。

```cpp
7|const char* ssid = "SSID";
9|const char* password = "PASSWORD";
```

URLを設定してください。

```cpp
29|// URL
30|const char *host_val = "http://" ;
31|const char *host_notice = "http://" ;
32|const char *host_flag = "http://" ;
33|const char *host_flag_last = "http://" ;
```

## 使い方

1. M5Stackにプログラムを書き込む
2. M5Stackを起動する
3. 水やりしたい植物の土にセンサーを刺す
4. ポンプのホースをうまい具合に接続する
5. 植物に名前をつける

## Related Repositories

- [Watering-Discord-Bot](https://github.com/hitto-hub/Watering-discord-bot)
- [Watering-M5Stack](https://github.com/hitto-hub/M5StackWatering) (this repository)
- [Watering-Back](https://github.com/hitto-hub/Watering-back)
