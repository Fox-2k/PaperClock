const fs = require("fs");

(async () => {

    const carFileNames = {
        '0' :"image_0.bmp",
        '1' :"image_1.bmp",
        '2' :"image_2.bmp",
        '3' :"image_3.bmp",
        '4' :"image_4.bmp",
        '5' :"image_5.bmp",
        '6' :"image_6.bmp",
        '7' :"image_7.bmp",
        '8' :"image_8.bmp",
        '9' :"image_9.bmp",
        ':' :"image_2p.bmp"
    }

    const points = 200
    const width = 136
    const height = 200

    const dummyCar = Array(width*height/8).fill("0x00")
    for(let i=0; i < dummyCar.length; i=i+width/8 )
        dummyCar[i] = "\n" + dummyCar[i]


    let data = `
#include "fonts.h"
#include <avr/pgmspace.h>

// Character bitmaps
const uint8_t Font${points}_Table[] PROGMEM = 
{
`

    // browse ascii table from " " to "~"
    for(let i = 0x20; i < 0x7F; i++) {
        console.log("char", String.fromCharCode(i))
        if(carFileNames[String.fromCharCode(i)]) {
            data += `// bitmap character for '${String.fromCharCode(i)}' ${await convert(carFileNames[String.fromCharCode(i)])}`
        }
        else
            data += `// dummy car for '${String.fromCharCode(i)}' ${dummyCar}`

        if(i < 126) data += ",\n"
    }

    data += `\n};
sFONT Font${points} = {
Font${points}_Table,
${width}, /* Width */
${height}, /* Height */
};
`

    await fs.promises.writeFile(`Font${points}.cpp`, data)


    async function convert(filename) {
        console.log(filename)
        let file = await fs.promises.readFile(filename)

        let blackTab = []
        let redTab = []

        let blackLine = []
        let redLine = []

        const offset = file.readInt32LE(10)
        let size = file.readInt32LE(02)
        let width = file.readInt32LE(18)
        let height = file.readInt32LE(22)
        console.log("size", size)
        console.log("offset", offset)
        console.log("width", width)
        console.log("height", height)

        
        for(let i = 0; i < (width*height); i=i+8) {
            let BlackByte = 0
            let RedByte = 0
            let cr = ""

            // At each new line, place the previous one on top of the created tab
            if(i == 0 || blackLine.length >= width/8) {
                blackTab = [...blackLine, ...blackTab]
                redTab = [...redLine, ...redTab]
                blackLine = []
                redLine = []
                cr = "\n"
            }
            
            
            for(let j=0; j<8; j++) {
                if(file.readUInt8(offset+i+j) != 0) {
                    BlackByte = BlackByte | (0x01 << (7-j));
                }
            }

            blackLine.push(cr + "0X" + ("0" + BlackByte.toString(16)).substr(-2) )
            redLine.push(cr + "0X" + ("0" + RedByte.toString(16)).substr(-2) )

        }

        blackTab = [...blackLine, ...blackTab]
        redTab = [...redLine, ...redTab]

        const epaperDataBlack = blackTab.join(",")
        const epaperDataRed = redTab.join(",")

        return `${epaperDataBlack.toUpperCase()}`
    }

    
    

})()