# koei_viewer
KOEI의 고전 게임인 삼국지 시리즈나 대항해시대 등의 게임의 이미지 데이터를 읽어 출력하는 프로그램

# 빌드
```sh
$ brew install sdl2
$ make
```

# 사용법
```sh
$ ./koei_viewer KAODATAP.S4 SAM4KAO.RGB
```

# 스크린샷
![sam3](https://user-images.githubusercontent.com/39606947/144691504-99f3e17a-2df5-431d-93fb-1fb3db41ea7b.png)
![sam4](https://user-images.githubusercontent.com/39606947/144691506-42bf728f-151e-46e9-97a0-312aa0860b8b.png)
![dh2](https://user-images.githubusercontent.com/39606947/144691511-48a18673-f0af-4594-ba8c-df35645d872a.png)
<img width="752" alt="hexichr" src="https://user-images.githubusercontent.com/39606947/144710552-b7c6b41d-3512-4f21-86e1-27b4a5ec606c.png">
<img width="752" alt="hexbchr" src="https://user-images.githubusercontent.com/39606947/144710556-a7218e88-7c2b-400a-bf75-427aa5051d6e.png">
<img width="752" alt="hexzchr" src="https://user-images.githubusercontent.com/39606947/144710562-85f6b579-6eec-4015-b215-0d0632609cae.png">
<img width="752" alt="sscchr2" src="https://user-images.githubusercontent.com/39606947/144710564-2dd660ff-bfbf-4373-b304-271951941d1e.png">


# 비고

* LS11로 시작하는 파일은 LS11압축이 되어 있는 파일이므로, 먼저 LS11 압축을 해제해야 함.
* 얼굴 데이터의 경우는 대부분 8색상을 쓰기 때문에, 3바이트당 8픽셀을 표현함. (각 바이트에서 1비트씩 가져와 하나의 픽셀값을 만듬)
