🌐 Hệ Thống Giám Sát Môi Trường Đa Tác Vụ Sử Dụng Kit CK-RA6M5 & FreeRTOS

Dự án này tập trung xây dựng một hệ thống nhúng thời gian thực (RTOS) dùng để thu thập thông số môi trường (nhiệt độ, độ ẩm), tự động đồng bộ thời gian thực (RTC) qua kết nối không dây và giám sát dữ liệu an toàn, phòng ngừa tranh chấp tài nguyên (Race Condition) bằng cơ chế Mutex trên dòng kit phát triển Renesas CK-RA6M5.

🖼️ Sơ Đồ Khối Luồng Hoạt Động (Data Flow)

Hệ thống hoạt động dựa trên sự phối hợp chặt chẽ giữa hai khối xử lý phần cứng thông qua các giao thức truyền thông nhúng tiêu chuẩn (I2C, UART):

       ┌────────────────────────┐
       │     Bộ Phát ESP32      │
       │ (Cung cấp Day-Time internet)
       └───────────┬────────────┘
                   │ 
                   │ UART (Truyền Day-Time)
                   ▼
┌───────────────────────────────────────────────┐
│              Renesas CK-RA6M5                 │
│  ┌─────────────────────────────────────────┐  │
│  │               FreeRTOS                  │  │
│  │                                         │  │
│  │  ┌─────────────────┐   g_sensor_mutex   │  │
│  │  │  Sensor_Thread  ├──────────┐         │  │
│  │  └────────┬────────┘          │         │  │
│  │           │ I2C               ▼         │  │
│  │           ▼             ┌───────────┐   │  │
│  │     ┌───────────┐       │   Mutex   │   │  │
│  │     │  Cảm biến │       └─────┬─────┘   │  │
│  │     │  HS3001   │             │         │  │
│  │     └───────────┘             ▼         │  │
│  │  ┌─────────────────┐   ┌────────────┐   │  │
│  │  │   Uart_Thread   ├──>│ Shared Var │   │  │
│  │  └────────┬────────┘   └────────────┘   │  │
│  │           │                             │  │
│  │           ▼                             │  │
│  │    ┌──────────────┐                     │  │
│  │    │  Ngoại vi    │                     │  │
│  │    │  Đồng bộ RTC │                     │  │
│  │    └──────────────┘                     │  │
│  └─────────────────────────────────────────┘  │
└──────────────────────┬────────────────────────┘
                       │ UART (Gửi JSON: Time + Temp + Hum)
                       ▼
         [Trạm Giám Sát / Gateway thu nhận]


🔌 Kiến Trúc Thiết Bị Phần Cứng

Thành phần

Thiết bị sử dụng

Vai trò kỹ thuật trong hệ thống

Khối điều khiển trung tâm

Renesas CK-RA6M5

Trang bị lõi ARM Cortex-M33 tần số hoạt động $f = 200 \text{ MHz}$. Quản lý tài nguyên, thực thi lõi điều hành FreeRTOS và đồng bộ hóa dữ liệu.

Khối cảm biến

Renesas HS3001

Cảm biến độ ẩm và nhiệt độ hiệu suất cao. Giao tiếp với MCU qua bus vật lý I2C.

Khối liên kết thời gian

ESP32

Lấy dữ liệu thời gian chính xác từ Internet (NTP Server) rồi truyền phát dạng chuỗi ký tự chuẩn qua UART để đồng bộ hóa.

⚙️ Hiện Thực Đa Nhiệm Trên FreeRTOS

Dự án phân chia bộ xử lý thành 2 tác vụ độc lập (Tasks) chạy song song dưới sự điều phối của Bộ lập lịch (Scheduler) FreeRTOS, bảo vệ dữ liệu bằng cơ chế loại trừ tương hỗ (Mutex).

🛡️ Cơ Chế Đồng Bộ Hóa (Shared Resources & Mutex)

Để lưu trữ dữ liệu đo được từ cảm biến, hệ thống sử dụng hai biến toàn cục chia sẻ giữa các luồng:

g_shared_temp (Lưu nhiệt độ)

g_shared_humi (Lưu độ ẩm)

Để tránh xung đột dữ liệu (Data Race) khi một tác vụ đang tiến hành ghi và tác vụ khác đang đọc, một khóa Mutex (g_sensor_mutex) được triển khai:

Chỉ tác vụ nắm giữ Mutex mới được quyền truy cập vào 2 biến toàn cục trên.

Tác vụ còn lại buộc phải đợi cho đến khi Mutex được giải phóng (nhả khóa).

🧵 Chi Tiết Các Luồng Tác Vụ (Threads)

1. Tác Vụ Đọc Cảm Biến (Sensor_Thread)

Hành vi luồng: 1. Thức dậy định kỳ mỗi $1000 \text{ ms}$ (sử dụng vTaskDelay(1000)).
2. Giao tiếp I2C với cảm biến HS3001 để đọc các giá trị đo thực tế.
3. Yêu cầu lấy khóa Mutex:

if (xSemaphoreTake(g_sensor_mutex, portMAX_DELAY) == pdTRUE) {
    g_shared_temp = temp_local;
    g_shared_humi = humi_local;
    xSemaphoreGive(g_sensor_mutex); // Giải phóng khóa
}


Trở về trạng thái Blocked để nhường CPU cho tác vụ khác.

2. Tác Vụ UART & Đồng Bộ RTC (Uart_Thread)

Hành vi luồng:

Pha 1 (Đồng bộ RTC nội): 1. Nhận chuỗi thông tin thời gian (Day-Time) từ module ESP32 truyền sang thông qua ngắt UART.
2. Sử dụng hàm giải mã chuỗi sscanf() để tách cấu trúc: Giờ, Phút, Giây, Ngày, Tháng, Năm.
3. Cập nhật trực tiếp vào thanh ghi của đồng hồ RTC nội vi của RA6M5 bằng cách gọi hàm driver:

R_RTC_CalendarTimeSet(&g_rtc_ctrl, &time_structure);


Pha 2 (Đóng gói & Phát dữ liệu):

Truy xuất thời gian thực hiện tại chính xác tuyệt đối từ RTC bằng hàm:

R_RTC_CalendarTimeGet(&g_rtc_ctrl, &current_time);


Chiếm giữ Mutex an toàn để lấy giá trị cảm biến mới nhất:

if (xSemaphoreTake(g_sensor_mutex, portMAX_DELAY) == pdTRUE) {
    temp_to_send = g_shared_temp;
    humi_to_send = g_shared_humi;
    xSemaphoreGive(g_sensor_mutex);
}


Định dạng dữ liệu thành chuỗi JSON và đẩy qua bộ truyền UART của RA6M5:
TX: {"time":"YYYY-MM-DD HH:MM:SS", "temp":XX.X, "hum":YY.Y}\r\n

Thực thi vTaskDelay(1000) để lặp lại chu kỳ tiếp theo.

📈 Giản Đồ Chuyển Cảnh Tác Vụ (Context Switch Timeline)

Dưới đây là tiến trình chuyển đổi ngữ cảnh thực tế của hệ thống được quản lý bởi FreeRTOS Scheduler:

Thời gian ──►
[Sensor_Thread] ──Đánh thức──► [Khóa Mutex] ──Đọc HS3001──► [Nhả Mutex] ──vTaskDelay(1000)──┐ (Blocked)
                                                                                          │
                                   [Context Switch điều phối bởi Scheduler] ◄──────────────┘
                                                                                          │
[Uart_Thread]   ◄─────────────────────────────────────────────────────────────────────────┘
                └──► [Khóa Mutex] ──Đọc Var Shared──► [Nhả Mutex] ──Gửi UART JSON──► vTaskDelay(1000)


📨 Định Dạng Gói Tin Đầu Raw (UART JSON Frame)

Gói tin đầu ra được thiết kế theo chuẩn JSON tinh gọn giúp các gateway hoặc hệ thống giám sát phía trên dễ dàng phân tích cú pháp (parse):

{
  "time": "2026-06-07 01:22:05",
  "temp": 28.7,
  "hum": 64.3
}


🛠️ Hướng Dẫn Cấu Hinh Trên Phần Mềm e2 Studio (FSP Config)

Để tái lập cấu hình hệ thống trên trình biên dịch e2 studio thông qua gói cấu hình FSP (Flexible Software Package), thực hiện các bước sau:

Khởi tạo Thread trên FreeRTOS Tab:

Thêm mới một Thread đặt tên là Sensor Thread (sensor_thread). Thiết lập mức ưu tiên (Priority) ở mức trung bình.

Thêm mới một Thread đặt tên là UART Thread (uart_thread). Thiết lập mức ưu tiên phù hợp.

Khởi tạo Mutex bảo vệ:

Trong cấu hình FreeRTOS, tạo một Semaphore dạng Mutex với tên định danh biến là g_sensor_mutex.

Thêm Stacks ngoại vi:

Tại Sensor Thread: Thêm Stack r_riic_master (giao tiếp I2C với cảm biến HS3001).

Tại UART Thread:

Thêm Stack r_rtc để quản lý và vận hành đồng hồ thời gian thực nội vi.

Thêm Stack r_sci_uart để nhận dữ liệu thời gian từ ESP32 và xuất gói tin JSON ra ngoài.

Generate Code & Biên dịch:

Nhấn nút Generate Project Content để sinh mã nguồn tự động cho các thư viện driver phần cứng, sau đó triển khai viết mã nguồn xử lý luồng trong các file sensor_thread_entry.c và uart_thread_entry.c.

eof
