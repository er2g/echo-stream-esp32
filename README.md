# esp32s3_uac_istiklal

Bu proje, Espressif'in `usb_device_uac` bileşeniyle, ESP32-S3 üzerinde _buffer kopmadan_, gerçek USB mikrofon gibi çalışan, **İstiklal Marşı melodisini** mikrofondan gönderen bir örnek kod içerir.

## Özellikler
- Tamamen Espressif dökümantasyonuna uygun UAC (USB Audio Class) altyapısı
- Mikrofon bilgisayarda "ESP USB Audio" olarak gözükür
- `main.c` içinde İstiklal Marşı'nın notaları döngüsel çalınır
- Menuconfig'ten örnekleme oranı ve interval ayarlanabilir

## Kullanım
1. ESP-IDF 5.5+ kurulu olmalı
2. Bağımlılıklar component manager ile otomatik çekilir (`espressif/usb_device_uac`)
3. Derle: idf.py set-target esp32s3
idf.py build flash monitor

4. PC'de "Ses Girişi"nden **'ESP USB Audio'**'yu seç, "Bu aygıtı dinle" de.

## Kod
Melodiyi `main.c` dosyasındaki `melody[]` dizisinden değiştirebilirsin.

---