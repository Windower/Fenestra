/*
 * Copyright © Windower Dev Team
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation files
 * (the "Software"),to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

namespace Windower.UI
{
    using System;
    using System.Diagnostics.CodeAnalysis;
    using System.Linq;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Media;
    using System.Windows.Media.Animation;

    public class TransitionControl : ContentControl
    {
        [SuppressMessage("Microsoft.Performance", "CA1810")]
        static TransitionControl() =>
            DefaultStyleKeyProperty.OverrideMetadata(typeof(TransitionControl),
                new FrameworkPropertyMetadata(typeof(TransitionControl)));

        public static readonly DependencyProperty EnterTransitionProperty =
            DependencyProperty.RegisterAttached("EnterTransition", typeof(string), typeof(TransitionControl),
                new PropertyMetadata(null));

        public static readonly DependencyProperty ExitTransitionProperty =
            DependencyProperty.RegisterAttached("ExitTransition", typeof(string), typeof(TransitionControl),
                new PropertyMetadata(null));

        public static string GetEnterTransition(DependencyObject obj)
        {
            if (obj == null)
            {
                throw new ArgumentNullException(nameof(obj));
            }

            return (string)obj.GetValue(EnterTransitionProperty);
        }

        public static void SetEnterTransition(DependencyObject obj, string value)
        {
            if (obj == null)
            {
                throw new ArgumentNullException(nameof(obj));
            }

            obj.SetValue(EnterTransitionProperty, value);
        }

        public static string GetExitTransition(DependencyObject obj)
        {
            if (obj == null)
            {
                throw new ArgumentNullException(nameof(obj));
            }

            return (string)obj.GetValue(ExitTransitionProperty);
        }

        public static void SetExitTransition(DependencyObject obj, string value)
        {
            if (obj == null)
            {
                throw new ArgumentNullException(nameof(obj));
            }

            obj.SetValue(ExitTransitionProperty, value);
        }

        private ContentPresenter PreviousContentHost;
        private ContentPresenter CurrentContentHost;

        private static readonly DependencyPropertyKey IsTransitioningKey =
            DependencyProperty.RegisterReadOnly(nameof(IsTransitioning), typeof(bool), typeof(TransitionControl),
                new PropertyMetadata(false, OnIsTransitioningChanged));

        public static readonly DependencyProperty DefaultTransitionProperty =
            DependencyProperty.Register(nameof(DefaultTransition), typeof(string), typeof(TransitionControl),
                new PropertyMetadata("SlideLeft"));

        public static readonly DependencyProperty AutoTransitionDirectionProperty =
            DependencyProperty.Register(nameof(AutoTransitionDirection), typeof(AutoTransitionDirection), typeof(TransitionControl),
                new PropertyMetadata(AutoTransitionDirection.None));

        public static readonly DependencyProperty AutoTransitionIndexProperty =
            DependencyProperty.Register(nameof(AutoTransitionIndex), typeof(int), typeof(TransitionControl),
                new PropertyMetadata(0, OnAutoTransitionIndexChanged));

        public static readonly DependencyProperty IsTransitioningProperty = IsTransitioningKey.DependencyProperty;

        public string DefaultTransition
        {
            get => (string)GetValue(DefaultTransitionProperty);
            set => SetValue(DefaultTransitionProperty, value);
        }

        public AutoTransitionDirection AutoTransitionDirection
        {
            get => (AutoTransitionDirection)GetValue(AutoTransitionDirectionProperty);
            set => SetValue(AutoTransitionDirectionProperty, value);
        }

        public int AutoTransitionIndex
        {
            get => (int)GetValue(AutoTransitionIndexProperty);
            set => SetValue(AutoTransitionIndexProperty, value);
        }

        public bool IsTransitioning
        {
            get => (bool)GetValue(IsTransitioningProperty);
            private set => SetValue(IsTransitioningKey, value);
        }

        public override void OnApplyTemplate()
        {
            base.OnApplyTemplate();
            PreviousContentHost = GetTemplateChild("PART_PreviousContentHost") as ContentPresenter;
            CurrentContentHost = GetTemplateChild("PART_CurrentContentHost") as ContentPresenter;

            if (CurrentContentHost != null)
            {
                CurrentContentHost.Content = Content;
            }

            if (VisualTreeHelper.GetChild(this, 0) is FrameworkElement root)
            {
                foreach (var group in VisualStateManager.GetVisualStateGroups(root).Cast<VisualStateGroup>())
                {
                    foreach (var state in group.States.Cast<VisualState>())
                    {
                        if (state.Name != "Normal")
                        {
                            state.Storyboard.Completed += (s, e) =>
                            {
                                if (((ClockGroup)s).CurrentState == ClockState.Filling)
                                {
                                    IsTransitioning = false;
                                }
                            };
                        }
                    }
                }
            }
        }

        private void OnIsTransitioningChanged(bool value)
        {
            if (!value && PreviousContentHost != null)
            {
                PreviousContentHost.Content = null;
            }
        }

        protected virtual void OnAutoTransitionIndexChanged(int oldIndex, int newIndex)
        {
            if (AutoTransitionDirection == AutoTransitionDirection.Horizontal)
            {
                if (newIndex > oldIndex)
                {
                    BeginTransition("SlideLeft");
                }
                else if (newIndex < oldIndex)
                {
                    BeginTransition("SlideRight");
                }
            }
            else if (AutoTransitionDirection == AutoTransitionDirection.Vertical)
            {
                if (newIndex > oldIndex)
                {
                    BeginTransition("SlideUp");
                }
                else if (newIndex < oldIndex)
                {
                    BeginTransition("SlideDown");
                }
            }
        }

        protected override void OnContentChanged(object oldContent, object newContent)
        {
            base.OnContentChanged(oldContent, newContent);
            if (PreviousContentHost != null && CurrentContentHost != null)
            {
                CurrentContentHost.Content = newContent;
                PreviousContentHost.Content = oldContent;
                if (AutoTransitionDirection == AutoTransitionDirection.None)
                {
                    BeginTransition(GetTransition(oldContent, newContent));
                }
            }
        }

        private void BeginTransition(string transition)
        {
            IsTransitioning = true;
            VisualStateManager.GoToState(this, "Normal", false);
            VisualStateManager.GoToState(this, transition, true);
        }

        private string GetTransition(object oldContent, object newContent)
        {
            string result = null;
            var oldDependencyObject = oldContent as DependencyObject;
            var newDependencyObject = newContent as DependencyObject;

            if (oldDependencyObject != null)
            {
                result = GetExitTransition(oldDependencyObject);
            }

            if (result == null && newDependencyObject != null)
            {
                result = GetEnterTransition(newDependencyObject);
            }

            return result ?? DefaultTransition;
        }

        private static void OnIsTransitioningChanged(DependencyObject d, DependencyPropertyChangedEventArgs e) =>
            ((TransitionControl)d).OnIsTransitioningChanged((bool)e.NewValue);

        private static void OnAutoTransitionIndexChanged(DependencyObject d, DependencyPropertyChangedEventArgs e) =>
            ((TransitionControl)d).OnAutoTransitionIndexChanged((int)e.OldValue, (int)e.NewValue);
    }
}
