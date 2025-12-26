/**
 * OTA固件更新服务器示例 (Node.js)
 * 
 * 功能：
 * 1. 检查固件更新接口 (/api/ota/check)
 * 2. 下载固件接口 (/api/ota/download)
 * 3. 网页界面上传固件文件
 * 
 * 安装依赖: npm install express multer
 * 运行: node ota_server.js
 */

const express = require('express');
const multer = require('multer');
const fs = require('fs');
const path = require('path');

const app = express();
const PORT = 80;

// 固件信息
let firmwareInfo = {
    version: '1.0.0',
    size: 0,
    filename: '',
    updateAvailable: false
};

// 配置文件上传
const storage = multer.diskStorage({
    destination: function (req, file, cb) {
        cb(null, './firmware/');
    },
    filename: function (req, file, cb) {
        cb(null, 'app.bin');
    }
});

const upload = multer({ storage: storage });

// 创建firmware目录
if (!fs.existsSync('./firmware')) {
    fs.mkdirSync('./firmware');
}

// 静态文件服务（网页）
app.use(express.static('public'));
app.use(express.json());

/* ==================== OTA API ==================== */

/**
 * 检查固件更新接口
 * GET /api/ota/check?device_id=xxx&current_version=1.0.0
 */
app.get('/api/ota/check', (req, res) => {
    const deviceId = req.query.device_id;
    const currentVersion = req.query.current_version;
    
    console.log(`[OTA检查] 设备: ${deviceId}, 当前版本: ${currentVersion}`);
    
    // 检查是否有新固件
    if (firmwareInfo.updateAvailable && firmwareInfo.version !== currentVersion) {
        console.log(`[OTA检查] 发现新版本: ${firmwareInfo.version}`);
        res.json({
            update_available: true,
            version: firmwareInfo.version,
            size: firmwareInfo.size,
            description: '新固件已上传'
        });
    } else {
        console.log(`[OTA检查] 无可用更新`);
        res.json({
            update_available: false,
            version: currentVersion
        });
    }
});

/**
 * 下载固件接口
 * GET /api/ota/download?device_id=xxx&offset=0
 */
app.get('/api/ota/download', (req, res) => {
    const deviceId = req.query.device_id;
    const offset = parseInt(req.query.offset) || 0;
    
    console.log(`[OTA下载] 设备: ${deviceId}, 偏移: ${offset}`);
    
    const firmwarePath = path.join(__dirname, 'firmware', 'app.bin');
    
    if (!fs.existsSync(firmwarePath)) {
        console.error('[OTA下载] 固件文件不存在');
        return res.status(404).json({ error: '固件文件不存在' });
    }
    
    // 读取固件文件
    const firmware = fs.readFileSync(firmwarePath);
    const chunkSize = 1024; // 每次发送1KB
    const end = Math.min(offset + chunkSize, firmware.length);
    const chunk = firmware.slice(offset, end);
    
    console.log(`[OTA下载] 发送数据: ${offset}-${end}/${firmware.length} bytes`);
    
    // 设置响应头
    res.setHeader('Content-Type', 'application/octet-stream');
    res.setHeader('Content-Length', chunk.length);
    
    // 发送数据块
    res.send(chunk);
});

/**
 * 上传固件接口（网页调用）
 * POST /api/ota/upload
 */
app.post('/api/ota/upload', upload.single('firmware'), (req, res) => {
    if (!req.file) {
        return res.status(400).json({ error: '未选择文件' });
    }
    
    const version = req.body.version || '1.1.0';
    const fileSize = req.file.size;
    
    console.log(`[固件上传] 版本: ${version}, 大小: ${fileSize} bytes`);
    
    // 更新固件信息
    firmwareInfo = {
        version: version,
        size: fileSize,
        filename: req.file.filename,
        updateAvailable: true
    };
    
    res.json({
        success: true,
        message: '固件上传成功',
        version: version,
        size: fileSize
    });
});

/**
 * 获取固件状态（网页调用）
 * GET /api/ota/status
 */
app.get('/api/ota/status', (req, res) => {
    res.json(firmwareInfo);
});

/* ==================== 启动服务器 ==================== */

app.listen(PORT, '0.0.0.0', () => {
    console.log('=====================================');
    console.log('  STM32 OTA固件更新服务器');
    console.log('=====================================');
    console.log(`服务器运行在: http://0.0.0.0:${PORT}`);
    console.log(`管理页面: http://localhost:${PORT}`);
    console.log('等待设备连接...\n');
});
