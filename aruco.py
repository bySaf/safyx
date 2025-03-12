import cv2
import numpy as np
import os
import pickle

# Файл, где будут сохранены параметры камеры после калибровки
CALIBRATION_FILE = "temo/camera_calibration.pkl"

def calibrate_camera(images, checkerboard_dims, square_size):
    # Параметры для калибровки камеры
    obj_points = []  # Мировые координаты 3D точек
    img_points = []  # Координаты 2D точек на изображении

    objp = np.zeros((checkerboard_dims[0] * checkerboard_dims[1], 3), np.float32)
    objp[:, :2] = np.mgrid[0:checkerboard_dims[0], 0:checkerboard_dims[1]].T.reshape(-1, 2) * square_size

    for image in images:
        gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
        ret, corners = cv2.findChessboardCorners(gray, checkerboard_dims, None)
        if ret:
            img_points.append(corners)
            obj_points.append(objp)

    ret, camera_matrix, dist_coeffs, rvecs, tvecs = cv2.calibrateCamera(
        obj_points, img_points, gray.shape[::-1], None, None
    )

    # Сохраняем калибровку в файл
    with open(CALIBRATION_FILE, 'wb') as f:
        pickle.dump((camera_matrix, dist_coeffs), f)

    return camera_matrix, dist_coeffs

def load_calibration():
    if os.path.exists(CALIBRATION_FILE):
        # Загружаем параметры из файла
        with open(CALIBRATION_FILE, 'rb') as f:
            camera_matrix, dist_coeffs = pickle.load(f)
        return camera_matrix, dist_coeffs
    else:
        return None, None

def capture_calibration_images(cap, checkerboard_dims):
    images = []
    print("Для калибровки камеры, сделайте несколько снимков шахматной доски. Нажмите 'c' для захвата.")
    while True:
        ret, frame = cap.read("kek.mp4")
        if not ret:
            break

        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        ret, corners = cv2.findChessboardCorners(gray, checkerboard_dims, None)

        if ret:
            cv2.drawChessboardCorners(frame, checkerboard_dims, corners, ret)

        cv2.imshow("Калибровка камеры", frame)

        key = cv2.waitKey(1)
        if key == ord('c'):  # Нажимаем 'c' для захвата изображения
            images.append(frame)
            print(f"Изображение захвачено. Всего: {len(images)}")
        elif key == ord('q'):  # Выход по клавише 'q'
            break

    cv2.destroyAllWindows()
    return images

def main():
    # Параметры шахматной доски для калибровки
    checkerboard_dims = (9, 6)  # Размеры шахматной доски (внутренние углы)
    square_size = 0.025  # Размер квадратов на доске в метрах (например, 2.5 см)

    # Загружаем параметры камеры, если они уже были сохранены
    camera_matrix, dist_coeffs = load_calibration()

    if camera_matrix is None or dist_coeffs is None:
        # Если параметров нет, начинаем калибровку
        print("Нет сохраненных данных калибровки. Начинаем калибровку...")
        cap = cv2.VideoCapture(0)
        if not cap.isOpened():
            print("Не удалось открыть камеру.")
            return

        # Снимаем изображения для калибровки
        images = capture_calibration_images(cap, checkerboard_dims)

        # Выполняем калибровку камеры
        camera_matrix, dist_coeffs = calibrate_camera(images, checkerboard_dims, square_size)

        cap.release()
        print("Калибровка завершена. Параметры сохранены.")
    else:
        print("Используем сохраненные параметры калибровки.")

    # Теперь можно использовать калиброванные параметры для дальнейшей работы
    aruco_dict = cv2.aruco.getPredefinedDictionary(cv2.aruco.DICT_6X6_250)
    aruco_params = cv2.aruco.DetectorParameters()

    marker_size = 0.05  # Размер маркера в метрах
    camera_world_pos = np.array([0, 0.21, 0], dtype=np.float32)

    cap = cv2.VideoCapture(0)
    if not cap.isOpened():
        print("Не удалось открыть камеру.")
        return

    while True:
        ret, frame = cap.read()
        if not ret:
            continue

        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

        # 1) Детектируем маркеры
        corners, ids, _ = cv2.aruco.detectMarkers(gray, aruco_dict, parameters=aruco_params)

        if ids is not None and len(ids) > 0:
            # 2) Оцениваем позу (rvec, tvec) каждого найденного маркера
            rvecs, tvecs, _ = cv2.aruco.estimatePoseSingleMarkers(corners, marker_size, camera_matrix, dist_coeffs)

            for i, corner in enumerate(corners):
                marker_id = ids[i][0]

                # Рисуем маркер и оси
                cv2.aruco.drawDetectedMarkers(frame, [corner], ids[i])
                cv2.drawFrameAxes(frame, camera_matrix, dist_coeffs, rvecs[i], tvecs[i], 0.03)

                t_camera = tvecs[i][0]  # Получаем позицию метки в системе координат камеры

                # 3) Расчет глобальных координат метки
                marker_world = camera_world_pos + t_camera

                # Считываем смещение по осям
                offset_x = t_camera[0]
                offset_y = t_camera[1]
                offset_z = t_camera[2]
                distance = np.linalg.norm(t_camera)  # Расстояние от камеры до маркера

                text = (
                        f"Offset X: {offset_x:.2f}m \nOffset Y: {offset_y:.2f}m \nOffset z: {offset_z:.2f}m \n")
                print(text)
                cX_marker = int(corner[0][0][0])
                cY_marker = int(corner[0][0][1]) - 15
                cv2.putText(frame, text, (cX_marker, cY_marker),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2)

        # Показываем кадр
        cv2.imshow("Frame", frame)

        # Выход по клавише 'q'
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    cap.release()
    cv2.destroyAllWindows()



if __name__ == "__main__":
    main()
