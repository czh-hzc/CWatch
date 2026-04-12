const WATCH_SERVICE_UUID = '00000000-0000-0000-0000-000000003412';
const WATCH_CHAR_UUID = '00000000-0000-0000-0000-00000000CDAB';
const WATCH_BLE_NAME = 'CWatch';
const API_KEY = '50c31e605bf04a1b881ef69774c253b2';
const HOST = 'p22tupvc6j.re.qweatherapi.com';

Page({
  data: {
    devices: [],
    connectedDeviceId: '',
    statusText: '未初始化',
    writeServiceId: '',
    writeCharId: '',
    autoConnecting: false,
    syncInProgress: false,
    syncStateText: '等待连接后自动同步',
    lastSyncText: '暂无同步记录',
    autoSyncDone: false
  },

  onLoad() {
    this._deviceFoundHandlerBound = false;
    this._bleValueChangeBound = false;
    this.startBluetooth(true);
  },

  startBluetooth(autoConnect = false) {
    wx.openBluetoothAdapter({
      success: () => {
        this.setData({ statusText: autoConnect ? '正在自动寻找手表...' : '蓝牙就绪，正在搜索...' });
        this.tryAutoConnectFromKnownDevices(() => this.startScan(autoConnect));
      },
      fail: (err) => {
        if (err.errCode === 10001) {
          wx.showToast({ title: '请打开手机蓝牙', icon: 'none' });
        } else {
          this.setData({ statusText: '初始化失败: ' + err.errMsg });
        }
      }
    });
  },

  startScan(autoConnect = false) {
    this.setData({ devices: [] });
    wx.startBluetoothDevicesDiscovery({
      allowDuplicatesKey: false,
      success: () => this.onDeviceFound(autoConnect)
    });
  },

  onDeviceFound(autoConnect = false) {
    if (this._deviceFoundHandlerBound) return;
    this._deviceFoundHandlerBound = true;

    wx.onBluetoothDeviceFound((res) => {
      res.devices.forEach(device => {
        if (!device.name && !device.localName) return;

        const foundDevices = this.data.devices.slice();
        const idx = foundDevices.findIndex(item => item.deviceId === device.deviceId);
        if (idx === -1) foundDevices.push(device);
        else foundDevices[idx] = device;
        this.setData({ devices: foundDevices });

        if (autoConnect && this.isWatchDevice(device)) {
          this.autoConnectToDevice(device);
        }
      });
    });
  },

  tryAutoConnectFromKnownDevices(fallback) {
    wx.getBluetoothDevices({
      success: (res) => {
        const knownDevice = (res.devices || []).find(device => this.isWatchDevice(device));
        if (knownDevice) {
          this.autoConnectToDevice(knownDevice, fallback);
          return;
        }
        fallback && fallback();
      },
      fail: () => {
        fallback && fallback();
      }
    });
  },

  isWatchDevice(device) {
    const name = (device.name || device.localName || '').trim();
    return name === WATCH_BLE_NAME;
  },

  autoConnectToDevice(device, onFail) {
    if (this.data.connectedDeviceId || this.data.autoConnecting) return;
    this.setData({
      autoConnecting: true,
      statusText: `正在自动连接 ${device.name || device.localName || WATCH_BLE_NAME}...`
    });
    this.connectByDevice(device.deviceId, device.name || device.localName || WATCH_BLE_NAME, true, onFail);
  },

  stopScan() {
    wx.stopBluetoothDevicesDiscovery({
      success: () => this.setData({ statusText: '已停止搜索' })
    });
  },

  connectDevice(e) {
    const deviceId = e.currentTarget.dataset.deviceId;
    const name = e.currentTarget.dataset.name;
    this.connectByDevice(deviceId, name || WATCH_BLE_NAME, false);
  },

  connectByDevice(deviceId, name, isAuto = false, onFail) {
    this.setData({ statusText: `正在连接 ${name}...` });
    this.stopScan();

    wx.createBLEConnection({
      deviceId,
      success: () => {
        this.setData({
          connectedDeviceId: deviceId,
          statusText: '已连接，正在获取通道...',
          autoConnecting: false,
          autoSyncDone: false
        });
        if (!isAuto) wx.showToast({ title: '连接成功', icon: 'success' });
        this.getDeviceServices(deviceId);
      },
      fail: () => {
        this.setData({
          statusText: isAuto ? '自动连接失败，可手动点击连接' : '连接失败',
          autoConnecting: false
        });
        onFail && onFail();
      }
    });
  },

  getDeviceServices(deviceId) {
    wx.getBLEDeviceServices({
      deviceId,
      success: () => this.getDeviceCharacteristics(deviceId, WATCH_SERVICE_UUID)
    });
  },

  getDeviceCharacteristics(deviceId, serviceId) {
    wx.getBLEDeviceCharacteristics({
      deviceId,
      serviceId,
      success: (res) => {
        const myChar = res.characteristics.find(item => item.uuid.toUpperCase() === WATCH_CHAR_UUID.toUpperCase());
        if (myChar) {
          if (myChar.properties.notify || myChar.properties.indicate) {
            wx.notifyBLECharacteristicValueChange({
              state: true,
              deviceId,
              serviceId,
              characteristicId: myChar.uuid,
              success: () => {
                if (!this._bleValueChangeBound) {
                  this._bleValueChangeBound = true;
                  wx.onBLECharacteristicValueChange((notifyRes) => {
                    console.log('【收到手表数据】', this.ab2hex(notifyRes.value));
                  });
                }
              }
            });
          }

          if (myChar.properties.write || myChar.properties.writeNoResponse) {
            this.setData({
              writeServiceId: serviceId,
              writeCharId: myChar.uuid,
              statusText: '通道就绪，正在自动同步数据...',
              syncStateText: 'BLE通道已就绪，开始自动同步'
            });
            if (!this.data.autoSyncDone) {
              this.startAutoSync();
            }
          }
        } else {
          this.setData({ statusText: '未找到匹配的 UUID 通道' });
        }
      }
    });
  },

  async startAutoSync() {
    if (this.data.syncInProgress) return;
    this.setData({
      syncInProgress: true,
      autoSyncDone: true,
      syncStateText: '正在同步时间...'
    });

    try {
      await this.syncTime({ silent: true });
      await this.delay(250);

      this.setData({ syncStateText: '正在同步今天天气...' });
      await this.getWeatherAndSend({ silent: true });
      await this.delay(300);

      this.setData({ syncStateText: '正在同步7天天气...' });
      await this.get7DayWeatherAndSend({ silent: true });
      await this.delay(300);

      this.setData({ syncStateText: '正在同步24小时天气...' });
      await this.get24HourWeatherAndSend({ silent: true });

      const finishedAt = this.formatTime(new Date());
      this.setData({
        syncInProgress: false,
        statusText: '连接完成，数据已自动同步',
        syncStateText: '自动同步完成',
        lastSyncText: `最近同步：${finishedAt}`
      });
      wx.showToast({ title: '自动同步完成', icon: 'success' });
    } catch (err) {
      this.setData({
        syncInProgress: false,
        statusText: err && err.message ? err.message : '自动同步失败',
        syncStateText: '自动同步失败，可手动重试'
      });
    }
  },

  syncTime(options = {}) {
    if (!this.checkConnection()) return;
    const now = new Date();
    const buffer = new ArrayBuffer(7);
    const dataView = new DataView(buffer);

    dataView.setUint16(0, now.getFullYear(), false);
    dataView.setUint8(2, now.getMonth() + 1);
    dataView.setUint8(3, now.getDate());
    dataView.setUint8(4, now.getHours());
    dataView.setUint8(5, now.getMinutes());
    dataView.setUint8(6, now.getSeconds());

    return this.sendToWatchPromise(buffer, `时间已同步: ${now.getHours()}:${now.getMinutes()}`, options);
  },

  getWeatherAndSend(options = {}) {
    if (!this.checkConnection()) return;
    this.setData({ statusText: '正在定位获取今天数据...' });
    return new Promise((resolve, reject) => {
      wx.getLocation({
        type: 'wgs84',
        success: (res) => {
          this.fetchWeatherAndAir(res.latitude, res.longitude, options).then(resolve).catch(reject);
        },
        fail: () => {
          this.setData({ statusText: '定位失败' });
          reject(new Error('定位失败'));
        }
      });
    });
  },

  fetchWeatherAndAir(lat, lon, options = {}) {
    return this.safeRequest(`https://${HOST}/geo/v2/city/lookup?location=${lon},${lat}&key=${API_KEY}`)
      .then(geoRes => {
        if (!geoRes.location) {
          this.setData({ statusText: '城市获取异常' });
          throw new Error('城市获取异常');
        }
        const cityName = geoRes.location[0].name;
        const cityId = geoRes.location[0].id;

        const p1 = this.safeRequest(`https://${HOST}/v7/weather/now?location=${cityId}&key=${API_KEY}`);
        const p2 = this.safeRequest(`https://${HOST}/v7/weather/3d?location=${cityId}&key=${API_KEY}`);
        const p3 = this.safeRequest(`https://${HOST}/airquality/v1/current/${lat}/${lon}?key=${API_KEY}`);

        return Promise.all([p1, p2, p3]).then(results => {
          const currentTemp = results[0].now ? parseInt(results[0].now.temp) : 0;
          const iconCode = results[0].now ? results[0].now.icon : '100';
          const maxTemp = (results[1].daily && results[1].daily[0]) ? parseInt(results[1].daily[0].tempMax) : 0;
          const minTemp = (results[1].daily && results[1].daily[0]) ? parseInt(results[1].daily[0].tempMin) : 0;

          let aqi = 0;
          if (results[2].indexes && results[2].indexes.length > 0) aqi = parseInt(results[2].indexes[0].aqi || results[2].indexes[0].value || 0);
          else if (results[2].now && results[2].now.aqi) aqi = parseInt(results[2].now.aqi || 0);

          this.setData({ statusText: `${cityName}: ${currentTemp}°C AQI:${aqi}` });
          return this.packAndSendDeluxeWeather(cityName, iconCode, currentTemp, maxTemp, minTemp, aqi, options);
        });
      });
  },

  packAndSendDeluxeWeather(cityName, iconCode, currentTemp, maxTemp, minTemp, aqi, options = {}) {
    const weatherEnum = this.parseIcon(iconCode);
    const cityBytes = this.strToUtf8Bytes(cityName).slice(0, 13);
    const buffer = new ArrayBuffer(7 + cityBytes.length);
    const dataView = new DataView(buffer);
    const uint8Array = new Uint8Array(buffer);

    dataView.setUint8(0, 0x02);
    dataView.setUint8(1, weatherEnum);
    dataView.setInt8(2, currentTemp);
    dataView.setInt8(3, maxTemp);
    dataView.setInt8(4, minTemp);
    dataView.setUint16(5, aqi, false);
    uint8Array.set(cityBytes, 7);

    return this.sendToWatchPromise(buffer, '今天天气已同步', options);
  },

  get7DayWeatherAndSend(options = {}) {
    if (!this.checkConnection()) return;
    this.setData({ statusText: '正在定位获取 7 天数据...' });
    return new Promise((resolve, reject) => {
      wx.getLocation({
        type: 'wgs84',
        success: (res) => {
          this.fetch7DayWeather(res.latitude, res.longitude, options).then(resolve).catch(reject);
        },
        fail: () => {
          this.setData({ statusText: '定位失败' });
          reject(new Error('定位失败'));
        }
      });
    });
  },

  fetch7DayWeather(lat, lon, options = {}) {
    return this.safeRequest(`https://${HOST}/geo/v2/city/lookup?location=${lon},${lat}&key=${API_KEY}`)
      .then(geoRes => {
        if (!geoRes.location) throw new Error('城市获取异常');
        return this.safeRequest(`https://${HOST}/v7/weather/7d?location=${geoRes.location[0].id}&key=${API_KEY}`);
      })
      .then(dailyRes => {
        if (dailyRes && dailyRes.daily && dailyRes.daily.length >= 7) {
          this.setData({ statusText: '7天数据拉取成功，打包中...' });
          return this.splitAndSend7DayWeather(dailyRes.daily, options);
        }
        throw new Error('7天数据获取异常');
      });
  },

  splitAndSend7DayWeather(dailyData, options = {}) {
    const parsedDays = dailyData.slice(0, 7).map(day => {
      return [this.parseIcon(day.iconDay), parseInt(day.tempMax), parseInt(day.tempMin)];
    });

    const buffer1 = new ArrayBuffer(14);
    const dv1 = new DataView(buffer1);
    dv1.setUint8(0, 0x03);
    dv1.setUint8(1, 0x00);
    for (let i = 0; i < 4; i++) {
      dv1.setUint8(2 + i * 3, parsedDays[i][0]);
      dv1.setInt8(3 + i * 3, parsedDays[i][1]);
      dv1.setInt8(4 + i * 3, parsedDays[i][2]);
    }

    const buffer2 = new ArrayBuffer(11);
    const dv2 = new DataView(buffer2);
    dv2.setUint8(0, 0x03);
    dv2.setUint8(1, 0x01);
    for (let i = 0; i < 3; i++) {
      dv2.setUint8(2 + i * 3, parsedDays[i + 4][0]);
      dv2.setInt8(3 + i * 3, parsedDays[i + 4][1]);
      dv2.setInt8(4 + i * 3, parsedDays[i + 4][2]);
    }

    return new Promise((resolve, reject) => {
      wx.writeBLECharacteristicValue({
        deviceId: this.data.connectedDeviceId,
        serviceId: this.data.writeServiceId,
        characteristicId: this.data.writeCharId,
        value: buffer1,
        success: () => {
          setTimeout(() => {
            this.sendToWatchPromise(buffer2, '7天天气分包发送成功', options).then(resolve).catch(reject);
          }, 150);
        },
        fail: () => reject(new Error('7天天气发送失败'))
      });
    });
  },

  get24HourWeatherAndSend(options = {}) {
    if (!this.checkConnection()) return;
    this.setData({ statusText: '正在定位获取 24 小时数据...' });
    return new Promise((resolve, reject) => {
      wx.getLocation({
        type: 'wgs84',
        success: (res) => {
          this.fetch24HourWeather(res.latitude, res.longitude, options).then(resolve).catch(reject);
        },
        fail: () => {
          this.setData({ statusText: '定位失败' });
          reject(new Error('定位失败'));
        }
      });
    });
  },

  fetch24HourWeather(lat, lon, options = {}) {
    return this.safeRequest(`https://${HOST}/geo/v2/city/lookup?location=${lon},${lat}&key=${API_KEY}`)
      .then(geoRes => {
        if (!geoRes.location) throw new Error('城市获取异常');
        return this.safeRequest(`https://${HOST}/v7/weather/24h?location=${geoRes.location[0].id}&key=${API_KEY}`);
      })
      .then(hourlyRes => {
        if (hourlyRes && hourlyRes.hourly && hourlyRes.hourly.length >= 24) {
          this.setData({ statusText: '24小时拉取成功，发送中...' });
          return this.splitAndSend24HourWeather(hourlyRes.hourly, options);
        }
        throw new Error('24小时数据获取异常');
      });
  },

  splitAndSend24HourWeather(hourlyData, options = {}) {
    const parsedHours = hourlyData.slice(0, 24).map(hour => {
      const timeHour = new Date(hour.fxTime).getHours();
      return [timeHour, this.parseIcon(hour.icon), parseInt(hour.temp)];
    });

    const sendPage = (page) => new Promise((resolve, reject) => {
      const buffer = new ArrayBuffer(20);
      const dv = new DataView(buffer);

      dv.setUint8(0, 0x04);
      dv.setUint8(1, page);

      for (let i = 0; i < 6; i++) {
        const globalIndex = page * 6 + i;
        dv.setUint8(2 + i * 3, parsedHours[globalIndex][0]);
        dv.setUint8(3 + i * 3, parsedHours[globalIndex][1]);
        dv.setInt8(4 + i * 3, parsedHours[globalIndex][2]);
      }

      setTimeout(() => {
        wx.writeBLECharacteristicValue({
          deviceId: this.data.connectedDeviceId,
          serviceId: this.data.writeServiceId,
          characteristicId: this.data.writeCharId,
          value: buffer,
          success: () => resolve(),
          fail: () => reject(new Error('24小时天气发送失败'))
        });
      }, page * 150);
    });

    return sendPage(0)
      .then(() => sendPage(1))
      .then(() => sendPage(2))
      .then(() => sendPage(3))
      .then(() => {
        this.setData({ statusText: '24小时分包发送成功' });
        if (!options.silent) wx.showToast({ title: '已同步', icon: 'success' });
      });
  },

  checkConnection() {
    if (!this.data.connectedDeviceId || !this.data.writeCharId) {
      wx.showToast({ title: '请先连接手表', icon: 'none' });
      return false;
    }
    return true;
  },

  sendToWatch(buffer, successMsg, options = {}) {
    return this.sendToWatchPromise(buffer, successMsg, options);
  },

  sendToWatchPromise(buffer, successMsg, options = {}) {
    return new Promise((resolve, reject) => {
      wx.writeBLECharacteristicValue({
        deviceId: this.data.connectedDeviceId,
        serviceId: this.data.writeServiceId,
        characteristicId: this.data.writeCharId,
        value: buffer,
        success: () => {
          this.setData({ statusText: successMsg });
          if (!options.silent) wx.showToast({ title: '已同步', icon: 'success' });
          resolve();
        },
        fail: () => reject(new Error('蓝牙写入失败'))
      });
    });
  },

  delay(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
  },

  formatTime(date) {
    const pad = (n) => `${n}`.padStart(2, '0');
    return `${pad(date.getHours())}:${pad(date.getMinutes())}:${pad(date.getSeconds())}`;
  },

  safeRequest(url) {
    return new Promise((resolve) => {
      wx.request({ url, success: (res) => resolve(res.data), fail: () => resolve({}) });
    });
  },

  parseIcon(iconCode) {
    const code = parseInt(iconCode);
    if (code >= 101 && code <= 104) return 0x01;
    if (code >= 300 && code < 400) return 0x02;
    if (code >= 400 && code < 500) return 0x03;
    return 0x00;
  },

  strToUtf8Bytes(str) {
    const bytes = [];
    for (let i = 0; i < str.length; i++) {
      const code = str.charCodeAt(i);
      if (code <= 0x7F) bytes.push(code);
      else if (code <= 0x7FF) {
        bytes.push((code >> 6) | 0xC0);
        bytes.push((code & 0x3F) | 0x80);
      } else {
        bytes.push((code >> 12) | 0xE0);
        bytes.push(((code >> 6) & 0x3F) | 0x80);
        bytes.push((code & 0x3F) | 0x80);
      }
    }
    return bytes;
  },

  ab2hex(buffer) {
    return Array.prototype.map.call(new Uint8Array(buffer), bit => ('00' + bit.toString(16)).slice(-2)).join(' ').toUpperCase();
  },

  onUnload() {
    if (this.data.connectedDeviceId) wx.closeBLEConnection({ deviceId: this.data.connectedDeviceId });
    wx.closeBluetoothAdapter();
  }
});
