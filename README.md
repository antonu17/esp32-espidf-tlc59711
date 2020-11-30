# esp32-espidf-tlc59711

how to upload web data:

### Prepare data dir

```
cd frontend
npm install
npm run build
cd ..
cp -a frontend/dist data
find data -name '*.map' -delete
```

### Upload SPIFFS

From platformio terminal run:

```
platformio run -t buildfs
platformio run -t uploadfs
```
