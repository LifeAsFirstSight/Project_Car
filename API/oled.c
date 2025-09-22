#include "stm32f1xx_hal.h"
#include "oled.h"
#include "oledfont.h"
#include "spi.h"  // 包含CubeMX生成的SPI外设头文件（需与实际生成文件名一致）


// 显存数组（128*64/8 = 1024字节，匹配规格书6-23章节128×64像素分辨率）
static uint8_t OLED_GRAM[128][8];

/**
 * @brief  OLED引脚初始化（仅初始化DC/CS引脚电平，SPI外设及GPIO已通过CubeMX配置）
 * @note   遵循规格书6-36章节SPI接口引脚功能定义，DC为数据/命令控制，CS为片选
 * @param  无
 * @retval 无
 */
void OLED_GPIO_Init(void)
{
    // 初始化DC/CS引脚电平，符合SPI通信初始状态（参考规格书6-56章节SPI时序要求）
    HAL_GPIO_WritePin(OLED_CS_PORT, OLED_CS_PIN, GPIO_PIN_SET);  // CS高电平：未选中OLED
    HAL_GPIO_WritePin(OLED_DC_PORT, OLED_DC_PIN, GPIO_PIN_RESET); // DC低电平：默认命令模式
}

/**
 * @brief  通过HAL库SPI外设写数据/命令（遵循规格书6-54/6-92章节SPI时序参数）
 * @param  date：要写入的数据/命令
 * @param  mode：0=命令，1=数据（通过DC引脚控制，匹配规格书6-36章节D/C#引脚功能）
 * @retval 无
 */
void OLED_WR_Byte(uint8_t date, uint8_t mode)
{
    // 设置DC电平：命令（0）或数据（1），满足规格书6-56章节t_AS（地址建立时间≥15ns）要求
    HAL_GPIO_WritePin(OLED_DC_PORT,OLED_DC_PIN, mode ? GPIO_PIN_SET : GPIO_PIN_RESET);
    
    // 选中OLED（CS拉低），满足规格书6-56章节t_CSS（片选建立时间≥20ns）要求
    HAL_GPIO_WritePin(OLED_CS_PORT,OLED_CS_PIN, GPIO_PIN_RESET);
    
    // 通过HAL库SPI发送函数传输数据，SPI参数（速率、极性、相位）已通过CubeMX配置
    // 匹配规格书6-56章节t_cycle（时钟周期≥100ns，对应SPI最大速率≤10MHz）要求
    HAL_SPI_Transmit(&hspi2, &date, 1, 100);  // hspi1需与CubeMX生成的SPI句柄一致，超时时间100ms
    
    // 取消选中OLED（CS拉高），满足规格书6-56章节t_CSH（片选保持时间≥20ns）要求
    HAL_GPIO_WritePin(OLED_CS_PORT,OLED_CS_PIN,GPIO_PIN_SET);
}

/**
 * @brief  OLED初始化（发送一系列初始化命令，遵循规格书6-209/6-227章节实际应用示例）
 * @param  无
 * @retval 无
 */
void OLED_Init(void)
{
    // 初始化DC/CS引脚电平（SPI外设已通过CubeMX初始化完成）
    OLED_GPIO_Init();
    
    // 上电时序：先上电VDD/VBAT，延迟100ms等待电源稳定（遵循规格书6-170章节Power up Sequence）
    HAL_Delay(100);
    
    // 发送初始化命令（匹配规格书6-209/6-227章节实际应用示例，适配128×64分辨率）
    OLED_WR_Byte(0xAE, 0); // 0xAE：关闭显示（Display Off，规格书6-167章节命令集参考SSD1315手册）
    OLED_WR_Byte(0xD3, 0); // 0xD3：设置显示偏移
    OLED_WR_Byte(0x00, 0); // 偏移量0（默认值，规格书6-209章节示例参数）
    OLED_WR_Byte(0x40, 0); // 0x40：设置显示起始行（起始行0，规格书6-209章节示例参数）
    OLED_WR_Byte(0xA1, 0); // 0xA1：段重映射（SEG0对应列地址127，适配屏幕显示方向）
    OLED_WR_Byte(0xC8, 0); // 0xC8：COM扫描方向（反向扫描，规格书6-209章节示例参数）
    OLED_WR_Byte(0xDA, 0); // 0xDA：设置COM引脚硬件配置
    OLED_WR_Byte(0x12, 0); // 硬件引脚配置（匹配规格书6-209章节COM引脚模式）
    OLED_WR_Byte(0x81, 0); // 0x81：设置对比度控制
    OLED_WR_Byte(0xB0, 0); // 对比度值（0xB0为典型值，规格书6-209章节示例参数，范围0x00~0xFF）
    OLED_WR_Byte(0xA4, 0); // 0xA4：正常显示（仅显示显存数据，非全亮模式，规格书6-167章节命令功能）
    OLED_WR_Byte(0xA6, 0); // 0xA6：正常显示（非反显模式，规格书6-167章节命令功能）
    OLED_WR_Byte(0xD5, 0); // 0xD5：设置显示时钟分频比/振荡器频率
    OLED_WR_Byte(0x90, 0); // 时钟分频比（规格书6-209章节示例参数，确保显示稳定）
    OLED_WR_Byte(0x8D, 0); // 0x8D：设置电荷泵（适配规格书6-101章节内部DC/DC电路配置）
    OLED_WR_Byte(0x14, 0); // 开启电荷泵（内部DC/DC使能，规格书6-126章节硬件配置要求）
    OLED_WR_Byte(0xAF, 0); // 0xAF：开启显示（Display On，规格书6-167章节命令集）
    
    OLED_Clear(); // 初始化后清屏，避免残留数据干扰
}

/**
 * @brief  更新显存到OLED屏幕（将OLED_GRAM数组数据写入屏幕，遵循规格书6-209章节显存操作逻辑）
 * @param  无
 * @retval 无
 */
void OLED_Refresh(void)
{
    uint8_t i, n;
    for (i = 0; i < 8; i++)
    {
        // 设置页地址（0~7，对应屏幕8个页，每页8行像素，匹配规格书6-24章节显存映射）
        OLED_WR_Byte(0xB0 + i, 0);
        // 设置列地址低4位（0x00~0x0F）
        OLED_WR_Byte(0x00, 0);
        // 设置列地址高4位（0x10~0x1F）
        OLED_WR_Byte(0x10, 0);
        // 连续写入当前页128列数据，通过HAL_SPI_Transmit实现批量传输
        HAL_GPIO_WritePin(OLED_DC_PORT, OLED_DC_PIN, GPIO_PIN_SET); // 切换为数据模式
        HAL_GPIO_WritePin(OLED_CS_PORT, OLED_CS_PIN, GPIO_PIN_RESET); // 选中OLED
        HAL_SPI_Transmit(&hspi2, &OLED_GRAM[0][i], 128, 500); // 批量传输128字节，超时时间500ms
        HAL_GPIO_WritePin(OLED_CS_PORT, OLED_CS_PIN, GPIO_PIN_SET); // 取消选中OLED
    }
}

/**
 * @brief  清屏（将显存全部置0，刷新后屏幕变黑，匹配规格书6-209章节初始化后清屏要求）
 * @param  无
 * @retval 无
 */
void OLED_Clear(void)
{
    uint8_t i, n;
    // 显存数组全部置0
    for (i = 0; i < 8; i++)
    {
        for (n = 0; n < 128; n++)
        {
            OLED_GRAM[n][i] = 0x00;
        }
    }
    OLED_Refresh(); // 刷新屏幕，显示清屏效果
}

/**
 * @brief  开启OLED显示（发送0xAF命令，参考规格书6-167章节命令集）
 * @param  无
 * @retval 无
 */
void OLED_Display_On(void)
{
    OLED_WR_Byte(0xAF, 0);
}

/**
 * @brief  关闭OLED显示（发送0xAE命令，参考规格书6-167章节命令集）
 * @param  无
 * @retval 无
 */
void OLED_Display_Off(void)
{
    OLED_WR_Byte(0xAE, 0);
}

/**
 * @brief  画点（在指定坐标(x,y)绘制点亮/熄灭状态，基于规格书6-24章节显存映射逻辑）
 * @param  x：横坐标（0~127）
 * @param  y：纵坐标（0~63）
 * @param  t：0=熄灭，1=点亮
 * @retval 无
 */
void OLED_DrawPoint(uint8_t x, uint8_t y, uint8_t t)
{
    uint8_t page = y / 8;    // 计算页号（0~7，每页8行像素）
    uint8_t bit = y % 8;     // 计算页内bit位（0~7）
    if (x < 128 && y < 64)   // 坐标范围判断（匹配屏幕分辨率）
    {
        if (t)
        {
            OLED_GRAM[x][page] |= (1 << bit);  // 点亮该点
        }
        else
        {
            OLED_GRAM[x][page] &= ~(1 << bit); // 熄灭该点
        }
    }
}

/**
 * @brief  画线（使用Bresenham算法在指定坐标(x1,y1)到(x2,y2)绘制直线）
 * @param  x1：起点横坐标（0~127）
 * @param  y1：起点纵坐标（0~63）
 * @param  x2：终点横坐标（0~127）
 * @param  y2：终点纵坐标（0~63）
 * @param  mode：0=熄灭，1=点亮
 * @retval 无
 */
#include "oledfont.h"

/**
 * @brief 在指定位置显示字符库中的字符（无返回值版本）
 * @param x: 字符左上角x坐标（0~127）
 * @param y: 字符左上角y坐标（0~63）
 * @param index: 要显示的字符索引（对应字符库中的Index[1]值）
 */
/**
 * @brief 在指定位置显示字符库中的字符
 * @param x: 字符左上角x坐标（0~127）
 * @param y: 字符左上角y坐标（0~63）
 * @param index: 要显示的字符索引（对应字符库中的Index[1]值）
 */
void OLED_DrawChar(uint8_t x, uint8_t y, uint8_t index)
{
    uint8_t i, j, k;
    uint8_t pixel;
    const typFNT_GB16_8 *pFont = NULL;  // 修正：使用const指针匹配字符库类型

    // 检查坐标是否超出屏幕范围（字符宽16px，高16px）
    if (x + 15 >= 128 || y + 15 >= 64)
        return;

    // 在字符库中查找对应索引的字符（字符库共16个字符）
    for (i = 0; i < 16; i++)
    {
        if (tfont16_8[i].Index[1] == index)  // 匹配索引值（Index[1]存储字符编码）
        {
            pFont = &tfont16_8[i];  // 现在类型匹配，无错误
            break;
        }
    }

    if (pFont == NULL)
        return;  // 未找到字符直接返回

    // 遍历字符点阵数据并绘制（16列x16行）
    for (i = 0; i < 16; i++)  // 遍历16列（x方向）
    {
        for (j = 0; j < 2; j++)  // 每列用2字节表示16行（分上下8行）
        {
            for (k = 0; k < 8; k++)  // 每个字节8个像素点（y方向）
            {
                // 获取当前像素点状态（1为亮，0为灭）
                pixel = (pFont->Msk[i * 2 + j] >> (7 - k)) & 0x01;
                
                // 调用画点函数绘制（计算实际坐标）
                OLED_DrawPoint(x + i, y + j * 8 + k, pixel);
            }
        }
    }
}

/**
 * @brief  显示字符串（基于OLED_ShowChar函数）
 * @param  x：横坐标（0~127）
 * @param  y：纵坐标（0~63）
 * @param  chr：字符串指针（ASCII字符）
 * @param  size1：字体大小（6x8=1，8x16=2）
 * @param  mode：0=反显关闭，1=反显开启
 * @retval 无
 */
//void OLED_ShowString(uint8_t x, uint8_t y, uint8_t *chr, uint8_t size1, uint8_t mode)
//{
//    while (*chr != '\0') // 遍历字符串直到结束符
//    {
//        OLED_ShowChar(x, y, *chr, size1, mode);
//        x += (size1 == 1) ? 6 : 8; // 6x8字体：字符宽度6像素；8x16字体：字符宽度8像素
//        if (x > 127) // 超出屏幕宽度，换行显示
//        {
//            x = 0;
//            y += (size1 == 1) ? 8 : 16;
//        }
//        chr++;
//    }
//}

/**
 * @brief  数字转换为字符串（辅助函数，用于OLED_ShowNum）
 * @param  num：要转换的数字
 * @param  len：数字长度
 * @param  buf：存储转换后字符串的缓冲区
 * @retval 无
 */
static void OLED_Num2Str(uint32_t num, uint8_t len, uint8_t *buf)
{
    buf[len] = '\0';
    for (int i = len - 1; i >= 0; i--)
    {
        buf[i] = (num % 10) + '0';
        num /= 10;
    }
}

/**
 * @brief  显示数字（整数）
 * @param  x：横坐标（0~127）
 * @param  y：纵坐标（0~63）
 * @param  num：要显示的数字（0~4294967295）
 * @param  len：数字长度（1~10）
 * @param  size1：字体大小（6x8=1，8x16=2）
 * @param  mode：0=反显关闭，1=反显开启
 * @retval 无
 */
//void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size1, uint8_t mode)
//{
//    uint8_t buf[11]; // 最大存储10位数字+结束符
//    OLED_Num2Str(num, len, buf);
//    OLED_ShowString(x, y, buf, size1, mode);
//}

// 以下函数可根据需求扩展实现（如汉字显示、图形绘制等，需匹配规格书硬件能力）
/**
 * @brief  显示汉字（需在oledfont.h中补充汉字字库，如16x16字库）
 * @param  x：横坐标（0~127）
 * @param  y：纵坐标（0~63）
 * @param  num：汉字在字库中的索引
 * @param  size1：字体大小（如16x16=2）
 * @param  mode：0=反显关闭，1=反显开启
 * @retval 无
 */
void OLED_ShowChinese(uint8_t x, uint8_t y, uint8_t num, uint8_t size1, uint8_t mode)
{
    // 需配合汉字字库实现，示例：16x16汉字需每次传输16字节数据，匹配显存页存储逻辑
    uint8_t i, j;
    if (size1 == 2) // 16x16字体（假设字库为chinese_1616[][32]）
    {
        for (i = 0; i < 16; i++)
        {
            // 需在oledfont.h中补充汉字字库数组，此处为示例逻辑
            // uint8_t date1 = chinese_1616[num][i*2];
            // uint8_t date2 = chinese_1616[num][i*2+1];
            for (j = 0; j < 8; j++)
            {
                // if (date1 & (1 << j)) OLED_DrawPoint(x+i, y+j, !mode);
                // else OLED_DrawPoint(x+i, y+j, mode);
                // if (date2 & (1 << j)) OLED_DrawPoint(x+i, y+j+8, !mode);
                // else OLED_DrawPoint(x+i, y+j+8, mode);
            }
        }
    }
}

/**
 * @brief  滚动显示（基于SSD1315滚动命令，参考规格书6-167章节命令集）
 * @param  num：滚动步数
 * @param  space：滚动间隔（ms
 * @param  mode：0=向右滚动，1=向左滚动
 * @retval 无
 */