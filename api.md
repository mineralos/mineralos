
# API DOCUMENTATION

  
# Authentication

You have two ways to authenticate with the miner:

1.  With username and password using Http Basic Auth
    
2.  With JWT obtained from the request /api/auth (JWT will expire if you reboot the miner or after 6 hours)
    

Recommended method is http basic auth, because it is not necessary to be regenerating the JWT.

# Api Calls

All API calls will receive a JSON response and a property “success” with true/false will indicate the result of the API call.

### POST /api/auth (authenticate with the miner)

#### Parameters:

-   username
    
-   password
    
#### JSON Response:

-   success: true or false
    
-   message: error message if credentials are not valid
    
-   jwt: JWT if credentials are valid
   
  

#### Example:

    {
    
    "success": true,
    
    "jwt": "eyJ0eXAiOiJKV1QiLCxhbGciOiJIUzI1NiJ9.eyJpc3MiOiJEcmFnb25NaW50IiwiaWF0IjoxNTIzMjg1NDMwLCJleHAiOjE1MjMzMDfwMzAsInVzZXIiOiJhZG1pbiJ9.hX5TocUdsfjilM92Sd-eUPd9X79fN2fvCNtFojIr8ho"
    
    }

  

### POST /api/summary (Fetch DEVS, POOLS and Fan Speed from the cgminer API)

-   This method require a JWT sent as a Bearer Token or a username and password sent using Http Basic Auth.
    

#### Parameters:

-   none
    

#### JSON Response:

-   success: true or false, will return false if cgminer is not ready yet)
    
-   DEVS: array with cgminer DEVS api call, each item of the array is a chain
    
-   POOLS: array with cgminer POOLS api call, each item of the array is a pool
    
-   HARDWARE: include an object named “Fan duty” with the fan speed
    

#### Response Example:

    {
    
    "success": true,
    
    "DEVS": [
    
    {
    
    "ASC": 0,
    
    "Name": "DT1",
    
    "ID": 0,
    
    "MHS av": 5384774.37,
    
    ...
    
    },
    
    {
    
    "ASC": 1,
    
    "Name": "DT1",
    
    "ID": 1,
    
    "MHS av": 5238461.62,
    
    ...
    
    },
    
    {
    
    "ASC": 2,
    
    "Name": "DT1",
    
    "ID": 2,
    
    "MHS av": 5282856.32,
    
    ...
    
    }
    
    ],
    
    "POOLS": [
    
    {
    
    "POOL": 0,
    
    "URL": "stratum+tcp://stratum.slushpool.com:3333",
    
    "Status": "Alive",
    
    "Priority": 0,
    
    ...
    
    }
    
    ],
    
    "HARDWARE": {
    
    "Fan duty": 80
    
    }
    
    }

  

### POST /api/overview (Fetch miner type, hardware information, network information and versions of the miner)

-   This method require a JWT sent as a Bearer Token or a username and password sent using Http Basic Auth.
    

#### Parameters:

-   none
    

#### JSON Response:

-   success: true or false
    
-   type: type of the miner
    
-   network:

>  dhcp: static or dhcp depending of the current network settings
> -   ipaddress: current IP address of the miner
> -   netmask: current netmask of the miner
>     
> -   gateway: current gateway of the miner
>     
> -   dns1: current primary DNS of the miner
>     
> -   dns2: current secondary DNS #2 of the miner

-   hardware:
    
> -   status: uptime of the miner
>     
> -   memUsed: memory used of the miner
>     
> -   memFree: memory available of the miner
>     
> -   memTotal: total memory of the miner
>     
> -   cacheUsefd: cached memory used of the miner
>     
> -   cacheFree: cached memory available of the miner
>     
> -   cacheTotal: total cache memory of the miner

    
-   version:

> -   hwver: model of the controller board
>     
> -   ethaddr: mac address of the ethernet port
>     
> -   build_date: date of when the installed firmware was built
>     
> -   platform_v: version of the installed firmware

    
  

#### Response Example:

    {
    
    "success": true,
    
    "type": "T1",
    
    "hardware": {
    
    "status": "15:38:31 up 10 min, load average: 0.30, 0.23, 0.11",
    
    "memUsed": 92300,
    
    "memFree": 158244,
    
    "memTotal": 250544,
    
    "cacheUsed": 72180,
    
    "cacheFree": 194592,
    
    "cacheTotal": 266772
    
    },
    
    "network": {
    
    "dhcp": "static",
    
    "ipaddress": "192.168.0.151",
    
    "netmask": "255.255.255.0",
    
    "gateway": "192.168.0.1",
    
    "dns1": "8.8.8.8",
    
    "dns2": "8.8.4.4"
    
    },
    
    "version": {
    
    "hwver": "g19",
    
    "ethaddr": "00:0a:35:00:00:00",
    
    "build_date": "9th of April 2018 03:20 PM",
    
    "platform_v": "t1_20180409_152051"
    
    }
    
    }

  

### POST /api/pools (Receive the configured pools of the miner)

-   This method require a JWT sent as a Bearer Token or a username and password sent using Http Basic Auth.
    

#### Parameters:

-   none
    

#### JSON Response:

-   success: true or false, will return false if cgminer is not ready yet
    
-   pools: array with pools object, each object will have a url, user, pass properties
    

  

#### Response Example:

    {
    
    "success": true,
    
    "pools": [
    
    {
    
    "url": "stratum+tcp://stratum.slushpool.com:3333",
    
    "user": "workner1.name",
    
    "pass": "x"
    
    }
    
    ]
    
    }

  
  

### POST /api/updatePools (Change the pools of the miner. This call will restart cgminer)

  

1.  This API should be executed by the user admin.
    
2.  This method require a JWT sent as a Bearer Token or a username and password sent using Http Basic Auth.
    

#### Parameters:

-   Pool1
    
-   UserName1
    
-   Password1
    
-   Pool2
    
-   UserName2
    
-   Password2
    
-   Pool3
    
-   UserName3
    
-   Password3
    

  

#### JSON Response:

-   success: true or false
    

  
  
  

### POST /api/updatePassword (Change the password of a user)

-   This API should be executed by the user admin.
    
-   This method require a JWT sent as a Bearer Token or a username and password sent using Http Basic Auth.
    

#### Parameters:

-   user: admin/guest
    
-   currentPassword: password of the admin user
    
-   newPassword: new password for the user sent in the user parameter
    

#### JSON Response:

-   success: true or false
    
-   message: error message)
    

  

### POST /api/network (Get the current network settings)

-   This method require a JWT sent as a Bearer Token or a username and password sent using Http Basic Auth.
    

#### Parameters:

-   none
    

#### JSON Response:

-   success: true or false
    
-   dhcp: static or dhcp depending of the current network settings
    
-   ipaddress: current IP address of the miner
    
-   netmask: current netmask of the miner
    
-   gateway: current gateway of the miner
    
-   dns1: current primary DNS of the miner
    
-   dns2: current secondary DNS #2 of the miner
    

  

#### Response Example:

    {
    
    "dhcp": "static",
    
    "ipaddress": "192.168.0.151",
    
    "netmask": "255.255.255.0",
    
    "gateway": "192.168.0.1",
    
    "dns1": "8.8.8.8",
    
    "dns2": "8.8.4.4",
    
    "success": true
    
    }

  

### POST /api/updateNetwork (Change the current network settings)

-   This API should be executed by the user admin.
    
-   This method require a JWT sent as a Bearer Token or a username and password sent using Http Basic Auth.
    

#### Parameters:

-   dhcp: static or dhcp
    
-   ipaddress: IP address of the miner (if dhcp parameter is static)
    
-   netmask: netmask of the miner (if dhcp parameter is static)
    
-   gateway: gateway of the miner (if dhcp parameter is static)
    
-   dns[]: array of DNS of the miner (if dhcp parameter is static)
    

Example: [“8.8.4.4”,”8.8.4.4”]

#### JSON Response:

-   success: true or false
    
-   message: error message if success false
    

  
  

### POST /api/type (Returns the type of the miner)

-   This method require a JWT sent as a Bearer Token or a username and password sent using Http Basic Auth.
    

#### Parameters:

-   none
    

#### JSON Response:

-   success: true or false
    
-   type: type of miner

#### Response Example:

    {
    
    "success": true,
    
    "type": "T1"
    
    }

  

### POST /api/reboot (Reboot the miner)

-   This API should be executed by the user admin.
    
-   This method require a JWT sent as a Bearer Token or a username and password sent using Http Basic Auth.
    
#### Parameters:

-   none
    
#### JSON Response:

-   success: true or false

### POST /api/poweroff (Power Off the Miner)

-   This API should be executed by the user admin.
    
-   This method require a JWT sent as a Bearer Token or a username and password sent using Http Basic Auth.
    
#### Parameters:

-   none
    
#### JSON Response:

-   success: true or false


### POST /api/restartCgMiner (Restart CGMiner)

-   This API should be executed by the user admin.
    
-   This method require a JWT sent as a Bearer Token or a username and password sent using Http Basic Auth.
    
#### Parameters:

-   none
    
#### JSON Response:

-   success: true or false
    

### POST /api/factoryReset (Remove all user settings and reboot the miner)

-   This API should be executed by the user admin.
    
-   This method require a JWT sent as a Bearer Token or a username and password sent using Http Basic Auth.
    

#### Parameters:

-   none
    

#### JSON Response:

-   success: true or false
    

### POST /api/getAutoTune (Returns cgminer auto-tune mode)

-   This API should be executed by the user admin.
    
-   This method require a JWT sent as a Bearer Token or a username and password sent using Http Basic Auth.
    

#### Parameters:

-   none
    

#### JSON Response:

-   success: true or false
    
-   autoTuneMode: ["efficient","balanced","factory","performance"]
    

  

#### Response Example:

    {
    
    "success": true,
    
    "autoTuneMode": "balanced"
    
    }

### POST /api/getAutoTuneStatus (Returns if cgminer Auto-Tune status)

-   This API should be executed by the user admin.
    
-   This method require a JWT sent as a Bearer Token or a username and password sent using Http Basic Auth.
    

#### Parameters:

-   none
    

#### JSON Response:

-   success: true or false
    
-   isRunning: true or false (cgminer is running or not)
-   isTuning: true or false (cgminer is tuning or not)
-   mode: ["efficient","balanced","factory","performance"]


### POST /api/setAutoTune (Set cgminer to use or not embedded auto-tune functionality)

-   This API should be executed by the user admin.
    
-   This method require a JWT sent as a Bearer Token or a username and password sent using Http Basic Auth.
    

#### Parameters:

-   autotune: ["efficient","balanced","factory","performance"]
    

#### JSON Response:

-   success: true or false
    

  
  

### POST /upgrade/upload (Upgrade the firmware of the miner)

-   This API should be executed by the user admin.
    
-   This method require a JWT sent as a Bearer Token or a username and password sent using Http Basic Auth.
    

#### Parameters:

-   upfile: Firmware file
    

#### JSON Response:

-   none (you can check the upgrade status with a web socket in ws://MINER_IP/upgrade/ws)

### POST /upgrade/download (Upgrade the firmware of the miner with a URL of the update file)

-   This API should be executed by the user admin.
    
-   This method require a JWT sent as a Bearer Token or a username and password sent using Http Basic Auth.
    

#### Parameters:

-   url: URL of the firmware
    

#### JSON Response:

-   none (you can check the upgrade status with a web socket in ws://MINER_IP/upgrade/ws)
    

### POST /api/getLatestFirmwareVersion (Returns the latest version available of the miner)

-   This method require a JWT sent as a Bearer Token or a username and password sent using Http Basic Auth.
    

#### Parameters:

-   none
    

#### JSON Response:

-   success: true or false
    
-   version: latest version available
    
-   versionDate: build date of the latest available version
    
-   url: URL where the latest version can be downloaded
    
-   info: message describing the changes made in the latest version
    
-   currentVersion: version installed in the miner
    
-   currentVersionDate: build date of version installed in the miner
    
-   isUpdated: true or false depending if the installed version is the same as the latest version available
    

  

#### Response Example:

    {
    
    "success": true,
    
    "version": "t1_20180409_010732",
    
    "versionDate": "9th of April 2018 01:07 AM",
    
    "url": "https://download.halongmining.com/files/t1/t1_20180409_010732.swu",
    
    "info": "important",
    
    "currentVersion": "t1_20180409_010732",
    
    "currentVersionDate": "9th of April 2018 01:07 AM",
    
    "isUpdated": true
    
    }

  
  
  

### POST /api/getDebugStats (Returns the cgminer stats of each board and each chip of the miner)

-   This method require a JWT sent as a Bearer Token or a username and password sent using Http Basic Auth.
    

#### Parameters:

-   none
    

#### JSON Response:

-   success: true or false
    
-   boards: array with each board of the miner
    

  

#### Response Example:

(too long to be display here)

### GET /stream/logs (Returns systemd-journald logs in chunked packages)

-   This method require a JWT sent as a Bearer Token or a username and password sent using Http Basic Auth.
    

#### Parameters:

-   none
    

#### Response:

logs lines in chunked mode
