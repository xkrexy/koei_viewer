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
<img width="752" alt="스크린샷 2021-12-05 23 34 19" src="https://user-images.githubusercontent.com/39606947/144751005-3e0a0746-6bdf-40cb-b945-405271d94ce8.png">
<img width="752" alt="스크린샷 2021-12-05 23 30 44" src="https://user-images.githubusercontent.com/39606947/144750859-d2defc8f-bc84-4a99-9988-e2b6db0353c2.png">
<img width="752" alt="스크린샷 2021-12-05 23 30 36" src="https://user-images.githubusercontent.com/39606947/144750863-eb2999ea-9419-45cf-8a5b-85d5e7b1e360.png">
<img width="752" alt="스크린샷 2021-12-05 23 30 27" src="https://user-images.githubusercontent.com/39606947/144750865-e92f943d-8477-48c9-9402-64be1e3fbb44.png">
<img width="752" alt="스크린샷 2021-12-05 23 30 22" src="https://user-images.githubusercontent.com/39606947/144750869-9c5406dc-8882-47fe-b826-53d3f6a5ea7a.png">



# 비고

* LS11로 시작하는 파일은 LS11압축이 되어 있는 파일이므로, 먼저 LS11 압축을 해제해야 함.
* 얼굴 데이터의 경우는 대부분 8색상을 쓰기 때문에, 3바이트당 8픽셀을 표현함. (각 바이트에서 1비트씩 가져와 하나의 픽셀값을 만듬)
