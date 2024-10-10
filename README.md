### PUBG 段位信息接口说明

#### 1. 服务器地址
- **地址1**: `http://pubg1.rump.asia:8088/`
- **地址2**: `...`


---

#### 2. 通过名称查询
- **接口地址**: `/api/pubg/getPlayerInfoByName`
- **请求方法**: GET
- **请求参数**:
  - `name`: 玩家游戏名

**示例**: 
```
http://pubg1.rump.asia:8088/api/pubg/getPlayerInfoByName?name=1iky-_-&seasons=31
```

---

#### 2. 通过ID查询
- **接口地址**: `/api/pubg/getPlayerInfoByAccountId`
- **请求方法**: GET
- **请求参数**:
  - `id`: 玩家 ID

**示例**: 
```
http://pubg1.rump.asia:8088/api/pubg/getPlayerInfoByAccountId?id=account.c0e530e9b7244b358def282782f893af
```

---

#### 3. 请求说明
- **请求方式**: HTTP GET
- **请求速率**: 100

---

#### 4. 官方接口
- **接口地址**: [https://documentation.pubg.com/en/api-keys.html](https://documentation.pubg.com/en/api-keys.html)
- **有兴趣的自己看看**

---


- **By QQ 2065816609**
