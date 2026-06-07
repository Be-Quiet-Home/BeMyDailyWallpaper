#ifndef DESKBAR_VIEW_H
#define DESKBAR_VIEW_H

#include <Rect.h>
#include <View.h>


class DeskbarView : public BView {
public:
	DeskbarView(BRect frame);

	virtual void AttachedToWindow();
	virtual void Draw(BRect updateRect);
};

#endif
