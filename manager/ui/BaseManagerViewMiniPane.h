﻿template<class M, class T, class U>
class BaseManagerViewUI;

template<class M, class T, class U>
class BaseManagerViewMiniPane :
	public juce::Component,
	public juce::Timer
{
public:
	BaseManagerViewMiniPane(BaseManagerViewUI<M, T, U>* managerUI);
	virtual ~BaseManagerViewMiniPane() {}

	BaseManagerViewUI<M, T, U>* managerUI;
	juce::ComponentAnimator ca;

	juce::Rectangle<float> paneViewBounds;
	juce::Rectangle<float> paneRealBounds;

	float showTime;

	virtual void paint(juce::Graphics& g) override;
	virtual void paintInternal(juce::Graphics& g) {}
	virtual void paintItem(juce::Graphics& g, U* ui);

	virtual void mouseEnter(const juce::MouseEvent& e) override;
	virtual void mouseExit(const juce::MouseEvent& e) override;
	virtual void mouseDown(const juce::MouseEvent& e) override;
	virtual void mouseDrag(const juce::MouseEvent& e) override;

	void updatePositionFromMouse();

	juce::Point<float> getPanePosForUIPos(juce::Point<int> viewPoint);
	juce::Point<float> getPanePosForViewPos(juce::Point<float> viewPoint);
	juce::Point<float> getViewPosForPanePos(juce::Point<float> panePoint);

	virtual void resized() override;
	virtual void updateContent();

	void timerCallback() override;

};

template<class M, class T, class U>
BaseManagerViewMiniPane<M, T, U>::BaseManagerViewMiniPane(BaseManagerViewUI<M, T, U>* managerUI) :
	managerUI(managerUI),
	showTime(1.5f)
{
	setAlpha(.1f);
	setOpaque(false);
	setInterceptsMouseClicks(true, true);
}

template<class M, class T, class U>
void BaseManagerViewMiniPane<M, T, U>::paint(juce::Graphics& g)
{
	g.fillAll(juce::Colours::white.withAlpha(.05f));
	g.setColour(juce::Colours::white.withAlpha(.2f));
	g.drawRect(getLocalBounds());

	if (managerUI->itemsUI.size() == 0) return;

	juce::Rectangle<int> mr = managerUI->getLocalBounds();
	juce::Rectangle<float> focusRect = managerUI->getViewBounds(mr);

	paneViewBounds = focusRect;

	for (auto& ui : managerUI->itemsUI)
	{
		paneViewBounds.setLeft(juce::jmin<float>(ui->item->viewUIPosition->x, paneViewBounds.getX()));
		paneViewBounds.setTop(juce::jmin<float>(ui->item->viewUIPosition->y, paneViewBounds.getY()));
		paneViewBounds.setRight(juce::jmax<float>(ui->item->viewUIPosition->x + ui->item->viewUISize->x, paneViewBounds.getRight()));
		paneViewBounds.setBottom(juce::jmax<float>(ui->item->viewUIPosition->y + ui->item->viewUISize->y, paneViewBounds.getBottom()));
	}

	juce::Rectangle<int> r = getLocalBounds();

	float itemsRatio = paneViewBounds.getWidth() * 1.0f / paneViewBounds.getHeight();
	float ratio = r.getWidth() * 1.0f / r.getHeight();

	if (itemsRatio > ratio)
	{
		float targetHeight = paneViewBounds.getWidth() / ratio;
		paneViewBounds.expand(0, (targetHeight - paneViewBounds.getHeight()) / 2);
	}
	else
	{
		float targetWidth = paneViewBounds.getHeight() * ratio;
		paneViewBounds.expand((targetWidth - paneViewBounds.getWidth()) / 2, 0);
	}

	paneRealBounds = managerUI->getBoundsInView(paneViewBounds).toFloat();

	paintInternal(g);

	for (auto& ui : managerUI->itemsUI)
	{
		paintItem(g, ui);
	}

	float fx = juce::jmap<float>(focusRect.getX(), paneViewBounds.getX(), paneViewBounds.getRight(), r.getX(), r.getRight());
	float fy = juce::jmap<float>(focusRect.getY(), paneViewBounds.getY(), paneViewBounds.getBottom(), r.getY(), r.getBottom());
	float fw = focusRect.getWidth() * r.getWidth() / paneViewBounds.getWidth();
	float fh = focusRect.getHeight() * r.getHeight() / paneViewBounds.getHeight();
	juce::Rectangle<int> focusPaneRect(fx, fy, fw, fh);

	g.setColour(BLUE_COLOR.withAlpha(.2f));
	g.fillRect(focusPaneRect);
	g.setColour(BLUE_COLOR.withAlpha(.5f));
	g.drawRect(focusPaneRect);
}

template<class M, class T, class U>
void BaseManagerViewMiniPane<M, T, U>::mouseEnter(const juce::MouseEvent& e)
{
	updateContent();
	stopTimer();
}

template<class M, class T, class U>
void BaseManagerViewMiniPane<M, T, U>::mouseExit(const juce::MouseEvent& e)
{
	startTimer(showTime * 1000);
}

template<class M, class T, class U>
void BaseManagerViewMiniPane<M, T, U>::mouseDown(const juce::MouseEvent& e)
{
	updatePositionFromMouse();
}


template<class M, class T, class U>
void BaseManagerViewMiniPane<M, T, U>::mouseDrag(const juce::MouseEvent& e)
{
	updatePositionFromMouse();
}

template<class M, class T, class U>
void BaseManagerViewMiniPane<M, T, U>::updatePositionFromMouse()
{
	juce::Point<float> p = this->getViewPosForPanePos(this->getMouseXYRelative().toFloat());
	juce::Rectangle<float> itemBounds = this->managerUI->getItemsViewBounds();
	juce::Point<float> cp = itemBounds.getConstrainedPoint(p);
	this->managerUI->manager->viewOffset = -cp.toInt();
	this->managerUI->resized();
	this->managerUI->repaint();
}



template<class M, class T, class U>
void BaseManagerViewMiniPane<M, T, U>::paintItem(juce::Graphics& g, U* ui)
{
	juce::Rectangle<int> b = ui->getBoundsInParent();

	juce::Point<float> bTopLeft = getPanePosForUIPos(b.getTopLeft());
	juce::Point<float> bBottomRight = getPanePosForUIPos(b.getBottomRight());

	juce::Rectangle<float> uiPaneBounds(bTopLeft, bBottomRight);

	juce::Colour ic = ui->item->itemColor != nullptr ? ui->item->itemColor->getColor() : juce::Colours::white;
	g.setColour(ic.brighter(.2f).withAlpha(.6f));
	g.fillRect(uiPaneBounds);
	g.setColour(ic.brighter(.5f).withAlpha(.8f));
	g.drawRect(uiPaneBounds);
}

template<class M, class T, class U>
juce::Point<float> BaseManagerViewMiniPane<M, T, U>::getPanePosForUIPos(juce::Point<int> uiPos)
{
	juce::Rectangle<float> r = getLocalBounds().toFloat();

	return juce::Point<float>(
		juce::jmap<float>(uiPos.x, paneRealBounds.getX(), paneRealBounds.getRight(), r.getX(), r.getRight()),
		juce::jmap<float>(uiPos.y, paneRealBounds.getY(), paneRealBounds.getBottom(), r.getY(), r.getBottom())
	);
}

template<class M, class T, class U>
juce::Point<float> BaseManagerViewMiniPane<M, T, U>::getPanePosForViewPos(juce::Point<float> viewPos)
{
	juce::Rectangle<float> r = getLocalBounds().toFloat();

	return juce::Point<float>(
		juce::jmap<float>(viewPos.x, paneViewBounds.getX(), paneViewBounds.getRight(), r.getX(), r.getRight()),
		juce::jmap<float>(viewPos.y, paneViewBounds.getY(), paneViewBounds.getBottom(), r.getY(), r.getBottom())
	);

}

template<class M, class T, class U>
juce::Point<float> BaseManagerViewMiniPane<M, T, U>::getViewPosForPanePos(juce::Point<float> panePos)
{
	juce::Rectangle<float> r = getLocalBounds().toFloat();

	return juce::Point<float>(
		juce::jmap<float>(panePos.x, r.getX(), r.getRight(), paneViewBounds.getX(), paneViewBounds.getRight()),
		juce::jmap<float>(panePos.y, r.getY(), r.getBottom(), paneViewBounds.getY(), paneViewBounds.getBottom())
	);

}

template<class M, class T, class U>
void BaseManagerViewMiniPane<M, T, U>::resized()
{
	updateContent();
}

template<class M, class T, class U>
void BaseManagerViewMiniPane<M, T, U>::updateContent()
{
	if ((!isVisible() || getAlpha() < 1) && !ca.isAnimating())
	{
		ca.fadeIn(this, 200);
	}

	if (!isMouseOverOrDragging()) startTimer(showTime * 1000);
	repaint();
}
template<class M, class T, class U>
void BaseManagerViewMiniPane<M, T, U>::timerCallback()
{
	ca.animateComponent(this, getBounds(), .1f, 200, false, 1, 1);
	stopTimer();
}
