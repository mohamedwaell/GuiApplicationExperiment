#include <Windows.h>
#include <dwmapi.h>
#include <windows.ui.composition.interop.h>

#include <winrt/windows.ui.h>
#include <winrt/windows.ui.composition.h>
#include <winrt/windows.ui.composition.desktop.h>

#include <winrt/windows.system.h>

using namespace winrt::Windows::UI;
using namespace winrt::Windows::System;
using namespace winrt::Windows::UI::Composition;
using namespace winrt::Windows::UI::Composition::Desktop;

using namespace std::chrono_literals;

Compositor g_compositor = nullptr;
DesktopWindowTarget g_target = nullptr;

int WinMain(HINSTANCE, HINSTANCE, LPSTR, INT)
{
	static const wchar_t* className = L"WindowClass";

	WNDCLASSW wndClass = {};
	wndClass.lpszClassName = className;
	wndClass.lpfnWndProc = DefWindowProcW;

	RegisterClassW(&wndClass);

	auto hwnd = CreateWindowExW(WS_EX_NOREDIRECTIONBITMAP,
								className,
								L"My Window",
								WS_OVERLAPPEDWINDOW | WS_VISIBLE, 
								CW_USEDEFAULT,
								CW_USEDEFAULT,
								CW_USEDEFAULT,
								CW_USEDEFAULT,
								NULL,
								NULL,
								NULL,
								NULL);

	auto value = TRUE;
	DwmSetWindowAttribute(hwnd, DWMWA_USE_HOSTBACKDROPBRUSH, &value, sizeof(value));

	auto controller = DispatcherQueueController::CreateOnDedicatedThread();
	auto dispatcher = controller.DispatcherQueue();

	dispatcher.TryEnqueue([hwnd]()
	{
		g_compositor = Compositor();
		winrt::com_ptr<ABI::Windows::UI::Composition::Desktop::ICompositorDesktopInterop> desktopInterop = g_compositor.as<ABI::Windows::UI::Composition::Desktop::ICompositorDesktopInterop>();

		desktopInterop->CreateDesktopWindowTarget(hwnd, false, (ABI::Windows::UI::Composition::Desktop::IDesktopWindowTarget**)winrt::put_abi(g_target));

		auto root = g_compositor.CreateSpriteVisual();
		root.RelativeSizeAdjustment({ 1, 1 });
		g_target.Root(root);

		auto brush = g_compositor.CreateHostBackdropBrush();
		root.Brush(brush);

		auto tintBrush = g_compositor.CreateColorBrush({ 100, 255, 0, 0 });
		auto tintVisual = g_compositor.CreateSpriteVisual();
		tintVisual.Opacity(0.24f);
		tintVisual.Brush(tintBrush);
		tintVisual.RelativeSizeAdjustment({ 1, 1 });
		root.Children().InsertAtTop(tintVisual);

		auto squareVisual = g_compositor.CreateSpriteVisual();
		squareVisual.Size({ 100, 100 });
		squareVisual.Offset({ 100, 100, 0 });
		squareVisual.CenterPoint({ 50, 50, 0 });
		root.Children().InsertAtTop(squareVisual);

		auto squareBrush = g_compositor.CreateColorBrush({ 255, 255, 255, 255 });
		squareVisual.Brush(squareBrush);

		auto offsetAnimation = g_compositor.CreateVector3KeyFrameAnimation();
		offsetAnimation.InsertKeyFrame(0, { 100, 100, 0 });
		offsetAnimation.InsertKeyFrame(0.5f, { 400, 400, 0 });
		offsetAnimation.InsertKeyFrame(1, { 100, 100, 0 });
		offsetAnimation.Duration(2s);
		offsetAnimation.IterationBehavior(AnimationIterationBehavior::Forever);

		auto rotationAnimation = g_compositor.CreateScalarKeyFrameAnimation();
		rotationAnimation.InsertKeyFrame(0, 0);
		rotationAnimation.InsertKeyFrame(0.5f, 360);
		rotationAnimation.InsertKeyFrame(1, 0);
		rotationAnimation.Duration(2s);
		rotationAnimation.IterationBehavior(AnimationIterationBehavior::Forever);

		auto opacityAnimation = g_compositor.CreateScalarKeyFrameAnimation();
		opacityAnimation.InsertKeyFrame(0, 1);
		opacityAnimation.InsertKeyFrame(0.5f, 0);
		opacityAnimation.InsertKeyFrame(1, 1);
		opacityAnimation.Duration(2s);
		opacityAnimation.IterationBehavior(AnimationIterationBehavior::Forever);

		auto colorAnimation = g_compositor.CreateColorKeyFrameAnimation();
		colorAnimation.InsertKeyFrame(0, { 100, 255, 0, 0 });
		colorAnimation.InsertKeyFrame(0.5f, { 255, 255, 0, 0 });
		colorAnimation.InsertKeyFrame(1, { 100, 255, 0, 0 });
		colorAnimation.Duration(2s);
		colorAnimation.IterationBehavior(AnimationIterationBehavior::Forever);
		colorAnimation.InterpolationColorSpace(CompositionColorSpace::HslLinear);

		squareVisual.StartAnimation(L"Offset", offsetAnimation);
		squareVisual.StartAnimation(L"RotationAngleInDegrees", rotationAnimation);
		squareVisual.StartAnimation(L"Opacity", opacityAnimation);
		tintBrush.StartAnimation(L"Color", colorAnimation);
	});

	MSG msg = {};
	while (GetMessageW(&msg, hwnd, NULL, NULL))
	{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	return 0;
}