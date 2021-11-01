#include <windows.h>
#include <ctime>
#include <cmath>
#include <vector>
#include <memory>
#include <algorithm>
#include <string>

#define K 4

#if K > 5
#error K must be in range[1,5]
#endif

#define PI 3.1415926535;

struct point_t {
	double x;
	double y;
	COLORREF color;
	int factor;
	point_t(double xx, double yy) { x = xx; y = yy; color = RGB(0, 0, 0); factor = 0; }
	point_t(double xx, double yy, COLORREF cr) { x = xx; y = yy; color = cr; factor = 0; }
	point_t(const point_t& pf, COLORREF cr) { x = pf.x; y = pf.y; color = cr; factor = 0; }
	point_t() { x = 0; y = 0; color = RGB(0, 0, 0); factor = 0; }
	bool operator==(const point_t& pf) { return this->x == pf.x && this->y == pf.y; }
	bool operator!=(const point_t& pf) { return !(*this == pf); }
};

point_t genRandomPoint(point_t center, float r) {
	const static int factor = 1000; /* 精度因子，越大生成的弧度和半径（浮点数）越精密，保持在 rand 函数参数阈值 (65536) 之内就行 */
	double rand_arc = (rand() % 1000 / 1000.0) * 2 * PI; /* 生成随机弧度 */
	double rand_r = (rand() % 1000 / 1000.0) * r;
	point_t pf;
	pf.x = cos(rand_arc) * rand_r + center.x;
	pf.y = sin(rand_arc) * rand_r + center.y;
	return pf;
}

inline double distance(const point_t& a, const point_t& b) {
	return sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}

std::vector<point_t> KMeansFirst(std::vector<point_t> & v, const std::vector<COLORREF> kcolors) {
	std::vector<point_t> rand_centers;
	int k = kcolors.size(), i, j;
	int siz = v.size();

	for (i = 0; i < k; ++i) {
		int rand_idx = rand() % siz;
		while (std::find(rand_centers.begin(), rand_centers.end(), v[rand_idx]) != rand_centers.end())
			rand_idx = rand() % siz;
		v[rand_idx].color = *(kcolors.begin() + i);
		rand_centers.push_back(v[rand_idx]);
	}

	for (j = 0; j < v.size(); ++j) {
		const point_t *min_center_p = &rand_centers[0];
		for (i = 1; i < k; ++i) // 计算最近的质心
			if (distance(v[j], rand_centers[i]) < distance(v[j], *min_center_p))
				min_center_p = &rand_centers[i];
		v[j].color = min_center_p->color;
	}

	return rand_centers;
}

std::vector<point_t> KMeans(const std::vector<point_t>& last_centers, std::vector<point_t>& v, const std::vector<COLORREF> kcolors, bool& convergence) {
	std::vector<point_t> calc_centers;
	int k = kcolors.size();
	point_t *uptr = new point_t[kcolors.size()];
	int i, j;

	for (j = 0; j < v.size(); ++j) {
		for (i = 0; i < k; ++i) {
			if (*(kcolors.begin() + i) == v[j].color) {
				uptr[i].x += v[j].x;
				uptr[i].y += v[j].y;
				uptr[i].color = v[j].color;
				uptr[i].factor += 1;
				break;
			}
		}
	}

	for (i = 0; i < k; ++i) {
		uptr[i].x /= uptr[i].factor;
		uptr[i].y /= uptr[i].factor;
		uptr[i].factor = 0;
		calc_centers.push_back(uptr[i]);
	}

	for (j = 0; j < v.size(); ++j) {
		const point_t* min_center_p = &calc_centers[0];
		for (int i = 1; i < k; ++i) // 计算最近的质心
			if (distance(v[j], calc_centers[i]) < distance(v[j], *min_center_p))
				min_center_p = &calc_centers[i];
		v[j].color = min_center_p->color;
	}

	for (i = 0; i < k; ++i) {
		point_t a = last_centers[i];
		point_t b = calc_centers[i];
		double dx = a.x - b.x;
		double dy = a.y - b.y;
		if (dx > 0.000001 || dy > 0.000001) {
			convergence = false;
			delete [] uptr;
			return calc_centers;
		}
	}

	convergence = true;
	delete [] uptr;
	return calc_centers;
}

void SetCenterPoint(HDC hdc, double x, double y, COLORREF color) {
	int ix = static_cast<int>(x * 20), iy = static_cast<int>(y * 20);
	HBRUSH hBrush = CreateSolidBrush(color);
	HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
	Ellipse(hdc, ix - 5, iy - 5, ix + 5, iy + 5);
	DeleteObject(SelectObject(hdc, hOldBrush));
}

void SetNormalPoint(HDC hdc, double x, double y, COLORREF color) {
	int ix = static_cast<int>(x * 20), iy = static_cast<int>(y * 20);
	HBRUSH hBrush = CreateSolidBrush(color);
	HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
	Rectangle(hdc, ix - 3, iy - 3, ix + 3, iy + 3);
	DeleteObject(SelectObject(hdc, hOldBrush));
}

void Line(HDC hdc, double x0, double y0, double x1, double y1, COLORREF color) {
	int ix0 = static_cast<int>(x0 * 20), iy0 = static_cast<int>(y0 * 20);
	int ix1 = static_cast<int>(x1 * 20), iy1 = static_cast<int>(y1 * 20);
	HPEN hPen = CreatePen(PS_DASHDOT, 1, color);
	HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
	MoveToEx(hdc, ix0, iy0, NULL);
	LineTo(hdc, ix1, iy1);
	DeleteObject(SelectObject(hdc, hOldPen));
}

COLORREF colors_arr[] = { RGB(255, 0, 0), RGB(0, 0, 255), RGB(255, 0, 255), RGB(255, 255, 0), RGB(0, 255, 0) };
std::vector<point_t> centers;

LRESULT CALLBACK WndProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam) {
	HDC hdc;
	PAINTSTRUCT ps;
	RECT rect;
	int i, j; 
	int cxClient, cyClient;
	static std::vector<point_t> v;
	static bool first_flag = true;
	static bool convergence = false;
	std::vector<COLORREF> colors;

	for (i = 0; i < K; i++) colors.push_back(colors_arr[i]);

	switch (umsg) {
	case WM_CREATE:
		for (i = 0; i < 10; ++i)
			v.push_back(point_t(genRandomPoint(point_t(0, 0), 5), 0));
		for (i = 0; i < 10; ++i)
			v.push_back(point_t(genRandomPoint(point_t(10, 0), 5), 0));
		for (i = 0; i < 10; ++i)
			v.push_back(point_t(genRandomPoint(point_t(0, 10), 5), 0));
		return 0;

	case WM_KEYUP:
		if (wParam == VK_RETURN) {
			if (first_flag) {
				centers = KMeansFirst(v, colors);
				first_flag = false;
			} else
				centers = KMeans(centers, v, colors, convergence);
			InvalidateRect(hwnd, NULL, true);
			if (convergence)
				MessageBox(hwnd, TEXT("所有质心已全部收敛！"), TEXT("信息"), MB_ICONINFORMATION);
		}
		break;

	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		GetClientRect(hwnd, &rect);
		cxClient = rect.right - rect.left;
		cyClient = rect.bottom - rect.top;

		SetMapMode(hdc, MM_LOENGLISH);
		SetViewportOrgEx(hdc, cxClient / 2, cyClient / 2, NULL);

		MoveToEx(hdc, -400, 0, NULL);
		LineTo(hdc, 400, 0);
		MoveToEx(hdc, 0, -400, NULL);
		LineTo(hdc, 0, 400);

		// 绘制连接线
		for (i = 0; i < centers.size(); ++i)
			for (j = 0; j < v.size(); ++j)
				if (v[j].color == centers[i].color)
					Line(hdc, centers[i].x, centers[i].y, v[j].x, v[j].y, RGB(100, 100, 100));
		// 绘制离散点
		for (i = 0; i < v.size(); ++i)
			SetNormalPoint(hdc, v[i].x, v[i].y, v[i].color);

		for (i = 0; i < centers.size(); ++i)
			SetCenterPoint(hdc, centers[i].x, centers[i].y, centers[i].color);

		
		EndPaint(hwnd, &ps);
		return 0;
		
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, umsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
	WNDCLASS wc;
	wc.hInstance = hInstance;
	wc.lpfnWndProc = WndProc;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.cbWndExtra = 0;
	wc.cbClsExtra = 0;
	wc.lpszClassName = TEXT("K-MEANS");
	wc.lpszMenuName = NULL;
	wc.style = CS_HREDRAW | CS_VREDRAW;

	if (!RegisterClass(&wc)) {
		MessageBox(NULL, TEXT("Cannot register window class!"), NULL, MB_ICONERROR);
		return 1;
	}

	srand(time(NULL));

	HWND hwnd = CreateWindow(TEXT("K-MEANS"), TEXT("K-Means Algorithm"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 800, NULL, NULL, hInstance, NULL);
	ShowWindow(hwnd, nShowCmd);
	UpdateWindow(hwnd);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}